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



#include "HelpWindow.h"

#include <iostream>

#include <ncurses.h>

#include <matchmaker/matchmaker.h>

#include "CompletionStack.h"
#include "word_filter.h"
#include "InputWindow.h"
#include "TitleOrientation.h"




std::string HelpWindow::title()
{
    static std::string const t{"Help"};
    return t;
}


TitleOrientation::Type HelpWindow::title_orientation() const
{
    return TitleOrientation::Center::grab();
}


void HelpWindow::resize_hook()
{
    height = (int) (root_y / 1.618 + 0.5);
    width = 63;
    y = (root_y - height) / 2;
    x = (root_x - width) / 2;
}


void HelpWindow::draw_hook()
{
    std::vector<std::string> content;
    content.push_back(std::string("             FEATURE    KEY(s)"));
    content.push_back(std::string("-------------------------------------------------------------"));
    content.push_back(std::string("        update input    type letters"));
    content.push_back(std::string("complete unambiguous    tab"));
    content.push_back(std::string(" change window focus    arrow left/right"));
    content.push_back(std::string("           scrolling    arrow up/down, page up/down, home/end"));
    content.push_back(std::string("    push input stack    Return"));
    content.push_back(std::string("     pop input stack    Del"));
    content.push_back(std::string("toggle filter window    any F key (F1..F12)"));
    content.push_back(std::string("                quit    Esc, ctrl + c"));
    content.push_back(std::string("          close help    Return"));

    int top_margin = 2;
    int i = 0;
    for (; i < (int) content.size() && i < height; ++i)
    {
        std::string const & line = content[i];

        int j = 0;
        for (; j < (int) line.size() && j < width - 2; ++j)
            mvwaddch(w, i + 1 + top_margin, j + 1, line[j]);

        // blank out rest of line
        for (; j < width - 2; ++j)
            mvwaddch(w, i + 1 + top_margin, j + 1, ' ');
    }

    // blank out remaining lines
    for (; i < height - 2 - top_margin; ++i)
        for (int j = 0; j < width - 2; ++j)
            mvwaddch(w, i + 1 + top_margin, j + 1, ' ');
}
