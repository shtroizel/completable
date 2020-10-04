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



#include "FilterWindow.h"

#include <iostream>

#include <ncurses.h>

#include "CompletionStack.h"
#include "InputWindow.h"
#include "Layer.h"
#include "word_filter.h"




FilterWindow::FilterWindow(CompletionStack & cs, WordStack & ws, InputWindow & iw, word_filter & f)
    : AbstractWindow(cs, ws)
    , input_win(iw)
    , wf(f)
{
}


FilterWindow::~FilterWindow()
{
}


std::string FilterWindow::title()
{
    static std::string const t{"Filter"};
    return t;
}


void FilterWindow::resize_hook()
{
    height = (int) (root_y / 1.618 + 0.5);
    width = (int) (root_x / 1.618 + 0.5);
    y = (root_y - height) / 2;
    x = (root_x - width) / 2;
}


void FilterWindow::post_resize_hook()
{
    keypad(w, true);
}


void FilterWindow::pre_disable_hook()
{
    // clear out the word stack
    while (!ws.empty())
        ws.pop();

    // clear out completion stack
    while (cs.count() > 1)
        cs.pop();
    cs.clear_top();

    input_win.mark_dirty();
}


void FilterWindow::draw_hook(bool)
{
    int top_margin = height - (int) ((height / 1.618 + word_attribute::variants().size() / 2.0) + 0.5);
    int indent = width - (int) (width / 1.618 + 0.5);

    static std::string const filter_type{"filter type: "};

    mvwprintw(w, top_margin - 2, indent - filter_type.length(), filter_type.c_str());

    if (hover == -1)
        wattron(w, A_BOLD);
    mvwprintw(w, top_margin - 2, indent, wf.direction.as_string().c_str());
    if (hover == -1)
        wattroff(w, A_BOLD);

    int i = 0;
    for (; i < (int) word_attribute::variants().size() && i + top_margin < height - 2; ++i)
    {
        auto att = word_attribute::from_index(i);
        std::string const & att_str = att.as_string();

        if (wf.attributes.is_set(att))
            wattron(w, A_REVERSE);

        if (hover == i)
            wattron(w, A_BOLD);

        int j = 0;
        for (; j < (int) att_str.length() && j + indent < width - 2; ++j)
            mvwaddch(w, i + top_margin, j + indent, att_str[j]);

        if (wf.attributes.is_set(att))
            wattroff(w, A_REVERSE);

        if (hover == i)
            wattroff(w, A_BOLD);

        // blank out rest of line
        for (; j + indent < width - 2; ++j)
            mvwaddch(w, i + top_margin, j + indent, ' ');
    }

    // blank out remaining lines
    for (; i + top_margin < height - 2; ++i)
        for (int j = 0; j < width - 2; ++j)
            mvwaddch(w, i + top_margin, j + 1, ' ');
}


void FilterWindow::on_KEY_UP()
{
    if (hover > -1)
    {
        --hover;
        mark_dirty();
    }
}


void FilterWindow::on_KEY_DOWN()
{
    if (hover < (int) word_attribute::variants().size() - 1)
    {
        ++hover;
        mark_dirty();
    }
}


void FilterWindow::on_RETURN()
{
    if (hover == -1)
    {
        int direction_index = wf.direction.as_index() + 1;
        if (direction_index >= (int) filter_direction::variants().size())
            direction_index = 0;
        wf.direction = filter_direction::from_index(direction_index);
    }
    else if (hover >= 0)
    {
        wf.attributes.toggle(word_attribute::from_index(hover));
    }
    else
    {
        std::cerr << "FilterWindow::toggle_hovered() :  hover [" << hover
                  << "] is outside valid range [-1.." << word_attribute::variants().size() << "]"
                  << std::endl;
    }

    mark_dirty();
}


Layer::Type FilterWindow::layer() const
{
    return Layer::F::grab();
}
