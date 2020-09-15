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


void CompletionWindow::draw_hook(CompletionStack const & cs)
{
    auto const & cur_completion = cs.top();
    int length = cur_completion.length - (cur_completion.display_start - cur_completion.start);
    for (int i = 0; i < length && i < height - 2; ++i)
    {
        std::string const & complete_entry = matchmaker::at(cur_completion.display_start + i);

        if (is_active() && i == 0)
            wattron(w, A_REVERSE);

        for (int j = 0; j < (int) complete_entry.size() && j < width - 2; ++j)
        {
            mvwaddch(
                w,
                i + 1,
                j + 1,
                complete_entry[j]
            );
        }

        if (is_active() && i == 0)
            wattroff(w, A_REVERSE);
    }
}


void CompletionWindow::on_KEY_UP(CompletionStack & cs)
{
    auto & c = cs.top();
    if (c.display_start > c.start)
        --c.display_start;
}


void CompletionWindow::on_KEY_DOWN(CompletionStack & cs)
{
    auto & c = cs.top();
    if (c.display_start < c.start + c.length - 1)
        ++c.display_start;
}


void CompletionWindow::on_PAGE_UP(CompletionStack & cs)
{
    auto & c = cs.top();
    c.display_start -= height - 2;
    if (c.display_start < c.start)
        c.display_start = c.start;
}


void CompletionWindow::on_PAGE_DOWN(CompletionStack & cs)
{
    auto & c = cs.top();
    c.display_start += height - 2;
    if (c.display_start >= c.start + c.length)
        c.display_start = c.start + c.length - 1;
}


void CompletionWindow::on_HOME(CompletionStack & cs)
{
    auto & c = cs.top();
    c.display_start = c.start;
}


void CompletionWindow::on_END(CompletionStack & cs)
{
    auto & c = cs.top();
    c.display_start = c.start + c.length - 1;
}
