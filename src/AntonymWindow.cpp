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



#include "AntonymWindow.h"

#include <ncurses.h>

#include <matchmaker/matchmaker.h>

#include "CompletionStack.h"
#include "CompletionWindow.h"
#include "LengthCompletionWindow.h"



AntonymWindow::AntonymWindow(CompletionWindow & cw, LengthCompletionWindow & lcw)
    : AbstractWindow(), len_completion_win(lcw)
{
    cw.add_dirty_dependency(this);
}


std::string const & AntonymWindow::title() const
{
    static std::string const t{"Antonyms"};
    return t;
}


void AntonymWindow::resize_hook()
{
    height = len_completion_win.get_height();
    width = len_completion_win.get_width() + root_x % 2;
    y = len_completion_win.get_y();
    x = len_completion_win.get_width();
}


void AntonymWindow::draw_hook(CompletionStack & cs)
{
    auto const & c = cs.top();
    auto const & antonyms = matchmaker::antonyms(c.display_start);

    int display_count = (int) antonyms.size() - c.ant_display_start;


    int i = 0;
    for (; i < display_count && i < height - 2; ++i)
    {
        std::string const & ant = matchmaker::at(antonyms[c.ant_display_start + i]);

        if (is_active() && i == 0)
            wattron(w, A_REVERSE);

        int j = 0;
        for (; j < (int) ant.length() && j < width - 2; ++j)
            mvwaddch(w, i + 1, j + 1, ant[j]);

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


void AntonymWindow::on_KEY_UP(CompletionStack & cs)
{
    auto & c = cs.top();
    if (c.ant_display_start > 0)
    {
        --c.ant_display_start;
        mark_dirty();
    }
}


void AntonymWindow::on_KEY_DOWN(CompletionStack & cs)
{
    auto & c = cs.top();
    if (c.ant_display_start < (int) matchmaker::antonyms(c.display_start).size() - 1)
    {
        ++c.ant_display_start;
        mark_dirty();
    }
}


void AntonymWindow::on_PAGE_UP(CompletionStack & cs)
{
    auto & c = cs.top();
    if (c.ant_display_start != 0)
    {
        c.ant_display_start -= height - 3;
        if (c.ant_display_start < 0)
            c.ant_display_start = 0;

        mark_dirty();
    }
}


void AntonymWindow::on_PAGE_DOWN(CompletionStack & cs)
{
    auto & c = cs.top();
    int const end = (int) matchmaker::antonyms(c.display_start).size() - 1;
    if (c.ant_display_start != end)
    {
        c.ant_display_start += height - 3;
        if (c.ant_display_start > end)
            c.ant_display_start = end;

        mark_dirty();
    }
}


void AntonymWindow::on_HOME(CompletionStack & cs)
{
    auto & c = cs.top();
    if (c.ant_display_start != 0)
    {
        c.ant_display_start = 0;
        mark_dirty();
    }
}


void AntonymWindow::on_END(CompletionStack & cs)
{
    auto & c = cs.top();
    int const end = matchmaker::antonyms(c.display_start).size() - 1;
    if (c.ant_display_start != end)
    {
        c.ant_display_start = end;
        mark_dirty();
    }
}


void AntonymWindow::on_RETURN_hook(CompletionStack & cs, WordStack & ws)
{
    auto const & antonyms = matchmaker::antonyms(cs.top().display_start);

    // do we even have antonyms?
    if (cs.top().ant_display_start >= (int) antonyms.size())
        return;

    std::string const & selected = matchmaker::at(antonyms[cs.top().ant_display_start]);

    // nothing to do?
    if (selected == cs.top().prefix)
        return;

    // save old prefix to word_stack
    ws.push(std::make_pair(cs.top().prefix, AbstractWindow::get_active_window()));

    // update completion stack
    while (cs.count() > 1)
        cs.pop();
    for (auto ch : selected)
        cs.push(ch);
}
