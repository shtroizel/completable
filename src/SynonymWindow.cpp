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



#include "SynonymWindow.h"

#include <ncurses.h>

#include <matchmaker/matchmaker.h>

#include "CompletionWindow.h"
#include "CompletionStack.h"
#include "InputWindow.h"
#include "LengthCompletionWindow.h"



SynonymWindow::SynonymWindow(CompletionWindow & cw) : completion_win(cw)
{
    completion_win.add_dirty_dependency(this);
}


std::string const & SynonymWindow::title() const
{
    static std::string const t{"Synonyms"};
    return t;
}


void SynonymWindow::resize_hook()
{
    height = completion_win.get_height();
    width = completion_win.get_width() + root_x % 2;
    y = completion_win.get_y();
    x = completion_win.get_width();
}


void SynonymWindow::draw_hook(CompletionStack & cs)
{
    auto const & c = cs.top();
    auto const & synonyms = matchmaker::synonyms(c.display_start);

    int display_count = (int) synonyms.size() - c.syn_display_start;

    int i = 0;
    for (; i < display_count && i < height - 2; ++i)
    {
        std::string const & syn = matchmaker::at(synonyms[c.syn_display_start + i]);

        if (is_active() && i == 0)
            wattron(w, A_REVERSE);

        int j = 0;
        for (; j < (int) syn.length() && j < width - 2; ++j)
            mvwaddch(w, i + 1, j + 1, syn[j]);

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


void SynonymWindow::on_KEY_UP(CompletionStack & cs)
{
    auto & c = cs.top();

    if (c.syn_display_start > 0)
    {
        --c.syn_display_start;
        mark_dirty();
    }
}


void SynonymWindow::on_KEY_DOWN(CompletionStack & cs)
{
    auto & c = cs.top();

    if (c.syn_display_start < (int) matchmaker::synonyms(c.display_start).size() - 1)
    {
        ++c.syn_display_start;
        mark_dirty();
    }
}


void SynonymWindow::on_PAGE_UP(CompletionStack & cs)
{
    auto & c = cs.top();

    if (c.syn_display_start != 0)
    {
        c.syn_display_start -= height - 3;
        if (c.syn_display_start < 0)
            c.syn_display_start = 0;
        mark_dirty();
    }
}


void SynonymWindow::on_PAGE_DOWN(CompletionStack & cs)
{
    auto & c = cs.top();

    if (matchmaker::synonyms(c.display_start).size() == 0)
        return;

    int const end = (int) matchmaker::synonyms(c.display_start).size() - 1;
    if (c.syn_display_start != end)
    {
        c.syn_display_start += height - 3;
        if (c.syn_display_start > end)
            c.syn_display_start = end;
        mark_dirty();
    }
}


void SynonymWindow::on_HOME(CompletionStack & cs)
{
    auto & c = cs.top();

    if (c.syn_display_start != 0)
    {
        c.syn_display_start = 0;
        mark_dirty();
    }
}


void SynonymWindow::on_END(CompletionStack & cs)
{
    auto & c = cs.top();

    if (matchmaker::synonyms(c.display_start).size() == 0)
        return;

    int const end = (int) matchmaker::synonyms(c.display_start).size() - 1;

    if (c.syn_display_start != end)
    {
        c.syn_display_start = end;
        mark_dirty();
    }
}


void SynonymWindow::on_RETURN_hook(CompletionStack & cs, WordStack & ws)
{
    auto const & synonyms = matchmaker::synonyms(cs.top().display_start);

    // do we even have synonyms?
    if (cs.top().syn_display_start >= (int) synonyms.size())
        return;

    std::string const & selected = matchmaker::at(synonyms[cs.top().syn_display_start]);

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
