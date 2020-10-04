/*
Copyright (c) 2020, Eric Hyer
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/



#include "AttributeWindow.h"

#include <iostream>

#include <ncurses.h>

#include "AntonymWindow.h"
#include "CompletionStack.h"
#include "CompletionWindow.h"
#include "Layer.h"
#include "LengthCompletionWindow.h"
#include "SynonymWindow.h"
#include "matchmaker.h"



AttributeWindow::AttributeWindow(
    CompletionStack & cs,
    WordStack & ws,
    CompletionWindow & cw,
    LengthCompletionWindow & lcw,
    SynonymWindow & sw,
    AntonymWindow & aw
)
    : AbstractWindow(cs, ws)
{
    cw.add_dirty_dependency(this);
    lcw.add_dirty_dependency(this);
    sw.add_dirty_dependency(this);
    aw.add_dirty_dependency(this);
}


std::string AttributeWindow::title()
{
    static std::string const t{"Attributes"};
    return t;
}


void AttributeWindow::resize_hook()
{
    height = 5;
    width = root_x;
    y = root_y - height;
    x = 0;
}


void AttributeWindow::draw_hook(bool)
{
    auto const & c = cs.top();
    if (c.standard_completion.size() == 0)
        return;

    int selection = c.standard_completion[c.display_start];
    int line = 1;

    {
        std::string const att_label{"     Attributes:"};
        mvwprintw(w, line, 1, att_label.c_str());

        static std::string const att_name = "name";
        static std::string const att_male_name = "male name";
        static std::string const att_female_name = "female name";
        static std::string const att_place = "place";
        static std::string const att_compound = "compound";
        static std::string const att_acronym = "acronym";

        std::vector<std::string const *> attributes;
        if (matchmaker::is_name(selection))        attributes.push_back(&att_name);
        if (matchmaker::is_male_name(selection))   attributes.push_back(&att_male_name);
        if (matchmaker::is_female_name(selection)) attributes.push_back(&att_female_name);
        if (matchmaker::is_place(selection))       attributes.push_back(&att_place);
        if (matchmaker::is_compound(selection))    attributes.push_back(&att_compound);
        if (matchmaker::is_acronym(selection))     attributes.push_back(&att_acronym);

        int indent = att_label.size() + 1;
        for (auto att : attributes)
        {
            if (width - indent <= (int) att->length() + 2)
                break;

            mvwprintw(w, line, indent, "  ");
            mvwprintw(w, line, indent + 2, att->c_str());
            indent += att->length() + 2;
        }

        for (; indent < width - 1; ++indent)
            mvwaddch(w, line, indent, ' ');
    }

    ++line;

    {
        std::vector<std::string const *> pos;
        matchmaker::parts_of_speech(selection, pos);

        std::string const pos_label{"Parts of Speech:"};
        mvwprintw(w, line, 1, pos_label.c_str());

        int indent = pos_label.size() + 1;
        for (auto p : pos)
        {
            if (width - indent <= (int) p->length() + 2)
            {
                for (; indent < width - 1; ++indent)
                    mvwaddch(w, line, indent, ' ');

                ++line;
                indent = pos_label.size() + 1;
            }
            if (line > 3)
            {
                line = 3;
                break;
            }

            mvwprintw(w, line, indent, "  ");
            mvwprintw(w, line, indent + 2, p->c_str());
            indent += p->length() + 2;
        }

        for (; line < 4; ++line)
        {
            for (; indent < width - 1; ++indent)
                mvwaddch(w, line, indent, ' ');

            indent = pos_label.size() + 1;
        }
    }
}


Layer::Type AttributeWindow::layer() const
{
    return Layer::Bottom::grab();
}
