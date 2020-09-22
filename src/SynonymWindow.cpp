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
#include "TitleOrientation.h"
#include "word_filter.h"



SynonymWindow::SynonymWindow(CompletionStack & cs, WordStack & ws, CompletionWindow & cw, word_filter & f)
    : AbstractWindow(cs, ws)
    , completion_win(cw)
    , wf(f)
{
    completion_win.add_dirty_dependency(this);
}


std::string SynonymWindow::title()
{
    std::string t{"Synonyms ("};

    std::vector<int> syn;
    get_synonyms(syn);
    t += std::to_string(syn.size());

    t += ")";

    return t;
}


TitleOrientation::Type SynonymWindow::title_orientation() const
{
    return TitleOrientation::Right::grab();
}


void SynonymWindow::resize_hook()
{
    height = completion_win.get_height();
    width = completion_win.get_width() + root_x % 2;
    y = completion_win.get_y();
    x = completion_win.get_width();
}


void SynonymWindow::draw_hook()
{
    auto & c = cs.top();
    if (c.standard_completion.size() == 0)
        return;

    std::vector<int> synonyms;
    get_synonyms(synonyms);

    // new filter could result in syn_display_start out of bounds so clamp
    if (c.syn_display_start >= (int) synonyms.size())
        c.syn_display_start = (int) synonyms.size() - 1;
    if (c.syn_display_start < 0)
        c.syn_display_start = 0;

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


void SynonymWindow::on_KEY_UP()
{
    auto & c = cs.top();

    if (c.syn_display_start > 0)
    {
        --c.syn_display_start;
        mark_dirty();
    }
}


void SynonymWindow::on_KEY_DOWN()
{
    std::vector<int> syn;
    get_synonyms(syn);
    if (syn.size() == 0)
        return;

    int const end = (int) syn.size() - 1;
    auto & c = cs.top();
    if (c.syn_display_start != end)
    {
        ++c.syn_display_start;

        if (c.syn_display_start > end)
            c.syn_display_start = end;

        mark_dirty();
    }
}


void SynonymWindow::on_PAGE_UP()
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


void SynonymWindow::on_PAGE_DOWN()
{
    std::vector<int> syn;
    get_synonyms(syn);
    if (syn.size() == 0)
        return;

    int const end = (int) syn.size() - 1;
    auto & c = cs.top();
    if (c.syn_display_start != end)
    {
        c.syn_display_start += height - 3;

        if (c.syn_display_start > end)
            c.syn_display_start = end;

        mark_dirty();
    }
}


void SynonymWindow::on_HOME()
{
    auto & c = cs.top();

    if (c.syn_display_start != 0)
    {
        c.syn_display_start = 0;
        mark_dirty();
    }
}


void SynonymWindow::on_END()
{
    std::vector<int> syn;
    get_synonyms(syn);
    if (syn.size() == 0)
        return;

    int const end = (int) syn.size() - 1;
    auto & c = cs.top();
    if (c.syn_display_start != end)
    {
        c.syn_display_start = end;
        mark_dirty();
    }
}


void SynonymWindow::on_RETURN_hook()
{
    std::vector<int> syn;
    get_synonyms(syn);

    if (syn.size() == 0)
        return;

    if (cs.top().syn_display_start >= (int) syn.size())
        cs.top().syn_display_start = (int) syn.size() - 1;

    std::string const & selected = matchmaker::at(syn[cs.top().syn_display_start]);

    // save old prefix to word_stack
    ws.push(std::make_pair(cs.top().prefix, AbstractWindow::get_active_window()));

    // update completion stack
    while (cs.count() > 1)
        cs.pop();
    for (auto ch : selected)
        cs.push(ch);
}


void SynonymWindow::get_synonyms(std::vector<int> & synonyms)
{
    synonyms.clear();
    auto & c = cs.top();

    if (c.standard_completion.size() == 0)
        return;

    if (c.display_start >= (int) c.standard_completion.size())
        c.display_start = (int) c.standard_completion.size() - 1;

    auto const & unfiltered = matchmaker::synonyms(c.standard_completion[c.display_start]);

    synonyms.reserve(unfiltered.size());
    for (auto syn : unfiltered)
        if (wf.passes(syn))
            synonyms.push_back(syn);
}
