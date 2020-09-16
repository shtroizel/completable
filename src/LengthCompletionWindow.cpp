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



#include "LengthCompletionWindow.h"

#include <ncurses.h>

#include <matchmaker/matchmaker.h>

#include "CompletionStack.h"
#include "CompletionWindow.h"
#include "InputWindow.h"


LengthCompletionWindow::LengthCompletionWindow(
    CompletionWindow const & cw,
    InputWindow & iw
)
    : AbstractWindow()
    , completion_win(cw)
    , input_win(iw)
{
    input_win.add_dirty_dependency(this);
}


LengthCompletionWindow::~LengthCompletionWindow()
{
}


std::string const & LengthCompletionWindow::title() const
{
    static std::string const t{"Length Completion"};
    return t;
}


void LengthCompletionWindow::resize_hook()
{
    y = completion_win.get_y() + completion_win.get_height();
    x = 0;
    height = root_y - y - 5;
    width = completion_win.get_width();
}


void LengthCompletionWindow::draw_hook(CompletionStack const & cs)
{
    auto const & cur_completion = cs.top();
    int length = cur_completion.length_completion.size() - cur_completion.len_display_start;
    if (length < 0)
        return;

    int long_index{0};

    int i = 0;
    for (; i < length && i < height - 2; ++i)
    {
        if (cur_completion.length_completion.size() > 0)
        {
            int length_completion_index = i + cur_completion.len_display_start;
            if (length_completion_index >= (int) cur_completion.length_completion.size())
                return;

            long_index = cur_completion.length_completion[length_completion_index];
        }
        else
            long_index = i;

        std::string const & complete_entry = matchmaker::at(matchmaker::by_longest()[long_index]);

        if (is_active() && i == 0)
            wattron(w, A_REVERSE);

        int j = 0;
        for (; j < (int) complete_entry.size() && j < width - 2; ++j)
            mvwaddch(w, i + 1, j + 1, complete_entry[j]);

        if (is_active() && i == 0)
            wattroff(w, A_REVERSE);

        // blank out rest of line
        for (; j < width - 2; ++j)
            mvwaddch(w, i + 1, j + 1, ' ');
    }

    // blank out remaining lines
    for (; i < height - 2; ++i)
        for (int j = 0; j < width - 2; ++j)
            mvwaddch(w, i + 1, j + 1, ' ');
}


void LengthCompletionWindow::on_KEY_UP(CompletionStack & cs)
{
    auto & c = cs.top();
    if (c.len_display_start > 0)
    {
        --c.len_display_start;
        mark_dirty();
    }
}


void LengthCompletionWindow::on_KEY_DOWN(CompletionStack & cs)
{
    auto & c = cs.top();
    if (c.len_display_start < (int) c.length_completion.size() - 1)
    {
        ++c.len_display_start;
        mark_dirty();
    }
}


void LengthCompletionWindow::on_PAGE_UP(CompletionStack & cs)
{
    auto & c = cs.top();
    if (c.len_display_start != 0)
    {
        c.len_display_start -= height - 3;
        if (c.len_display_start < 0)
            c.len_display_start = 0;

        mark_dirty();
    }
}


void LengthCompletionWindow::on_PAGE_DOWN(CompletionStack & cs)
{
    auto & c = cs.top();
    int const end = (int) c.length_completion.size() - 1;
    if (c.len_display_start != end)
    {
        c.len_display_start += height - 3;
        if (c.len_display_start > end)
            c.len_display_start = end;

        mark_dirty();
    }
}


void LengthCompletionWindow::on_HOME(CompletionStack & cs)
{
    auto & c = cs.top();
    if (c.len_display_start != 0)
    {
        c.len_display_start = 0;
        mark_dirty();
    }
}


void LengthCompletionWindow::on_END(CompletionStack & cs)
{
    auto & c = cs.top();
    int const end = c.length_completion.size() - 1;
    if (c.len_display_start != end)
    {
        c.len_display_start = end;
        mark_dirty();
    }
}
