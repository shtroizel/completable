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



#include "CompletionWindow.h"

#include <ncurses.h>

#include <matchmaker/matchmaker.h>

#include "CompletionStack.h"
#include "InputWindow.h"



CompletionWindow::CompletionWindow(CompletionStack & cs, WordStack & ws, InputWindow & iw)
    : AbstractWindow::AbstractWindow(cs, ws)
    , input_win{iw}
{
    AbstractWindow::set_active_window(this);
    input_win.add_dirty_dependency(this);
}


CompletionWindow::~CompletionWindow()
{
}


std::string const & CompletionWindow::title() const
{
    static std::string const t{"Completion"};
    return t;
}


void CompletionWindow::resize_hook()
{
    int combined_height = root_y - 5 - y;
    height = y + ((combined_height * 9) / 17);
    width = root_x / 2;
    y = 3;
    x = 0;
}


void CompletionWindow::draw_hook()
{
    auto const & cur_completion = cs.top();
    int display_count = cur_completion.length - (cur_completion.display_start - cur_completion.start);

    int i = 0;
    for (; i < display_count && i < height - 2; ++i)
    {
        std::string const & complete_entry = matchmaker::at(cur_completion.display_start + i);

        if (i == 0)
        {
            if (is_active())
                wattron(w, A_REVERSE);
            else
                wattron(w, A_BOLD);
        }

        int j = 0;
        for (; j < (int) complete_entry.size() && j < width - 2; ++j)
            mvwaddch(w, i + 1, j + 1, complete_entry[j]);

        if (i == 0)
        {
            if (is_active())
                wattroff(w, A_REVERSE);
            else
                wattroff(w, A_BOLD);
        }

        // blank out rest of line
        for (; j < width - 2; ++j)
            mvwaddch(w, i + 1, j + 1, ' ');
    }

    // blank out remaining lines
    for (; i < height - 2; ++i)
        for (int j = 0; j < width - 2; ++j)
            mvwaddch(w, i + 1, j + 1, ' ');
}


void CompletionWindow::on_KEY_UP()
{
    auto & c = cs.top();
    if (c.display_start > c.start)
    {
        --c.display_start;
        mark_dirty();
    }
}


void CompletionWindow::on_KEY_DOWN()
{
    auto & c = cs.top();
    if (c.display_start < c.start + c.length - 1)
    {
        ++c.display_start;
        mark_dirty();
    }
}


void CompletionWindow::on_PAGE_UP()
{
    auto & c = cs.top();
    if (c.display_start != c.start)
    {
        c.display_start -= height - 3;
        if (c.display_start < c.start)
            c.display_start = c.start;
        mark_dirty();
    }
}


void CompletionWindow::on_PAGE_DOWN()
{
    auto & c = cs.top();
    int const end = c.start + c.length - 1;
    if (c.display_start != end)
    {
        c.display_start += height - 3;
        if (c.display_start > end)
            c.display_start = end;
        mark_dirty();
    }
}


void CompletionWindow::on_HOME()
{
    auto & c = cs.top();
    if (c.display_start != c.start)
    {
        c.display_start = c.start;
        mark_dirty();
    }
}


void CompletionWindow::on_END()
{
    auto & c = cs.top();
    int const end = c.start + c.length - 1;
    if (c.display_start != end)
    {
        c.display_start = end;
        mark_dirty();
    }
}


void CompletionWindow::on_RETURN_hook()
{
    auto & c = cs.top();

    std::string const & selected = matchmaker::at(c.display_start);

    if (selected == c.prefix)
        return;

    ws.push(std::make_pair(c.prefix, AbstractWindow::get_active_window()));

    for (auto i = c.prefix.length(); i < selected.length(); ++i)
        cs.push(selected[i]);
}
