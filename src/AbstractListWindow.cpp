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



#include "AbstractListWindow.h"

#include <ncurses.h>

#include <matchmaker/matchmaker.h>

#include "CompletionStack.h"
#include "InputWindow.h"
#include "word_filter.h"



AbstractListWindow::AbstractListWindow(
    CompletionStack & cs,
    WordStack & ws,
    InputWindow & iw,
    word_filter & f
)
    : AbstractWindow(cs, ws)
    , input_win(iw)
    , wf(f)
{
    iw.add_dirty_dependency(this);
}


void AbstractListWindow::draw_hook()
{
    auto & c = cs.top();
    if (c.standard_completion.size() == 0)
        return;

    auto words = filtered_words();

    // new filter could result in display_start out of bounds so clamp
    if (display_start() >= (int) words.size())
        display_start() = (int) words.size() - 1;
    if (display_start() < 0)
        display_start() = 0;

    int display_count = (int) words.size() - display_start();

    int i = 0;
    for (; i < display_count && i < height - 2; ++i) // 2 for borders (top, bottom)
    {
        std::string const & word = string_from_index(words[display_start() + i]);

        // highlight first word
        if (is_active() && i == 0)
            wattron(w, A_REVERSE);

        int j = 0;
        for (; j < (int) word.length() && j < width - 2; ++j) // 2 for borders (left, right)
            mvwaddch(w, i + 1, j + 1, word[j]);

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


void AbstractListWindow::on_KEY_UP()
{
    int & ds = display_start();
    if (ds > 0)
    {
        --ds;
        mark_dirty();
        cache_dirty.set_false();
    }
}


void AbstractListWindow::on_KEY_DOWN()
{
    int & ds = display_start();

    cache_dirty.set_false();
    auto words = filtered_words();
    if (words.size() == 0)
        return;

    int const end = (int) words.size() - 1;
    if (ds != end)
    {
        ++ds;

        if (ds > end)
            ds = end;

        mark_dirty();
        cache_dirty.set_false();
    }
}


void AbstractListWindow::on_PAGE_UP()
{
    int & ds = display_start();
    if (ds != 0)
    {
        ds -= height - 3; // 2 for borders, 1 for context

        if (ds < 0)
            ds = 0;

        mark_dirty();
        cache_dirty.set_false();
    }
}


void AbstractListWindow::on_PAGE_DOWN()
{
    int & ds = display_start();

    cache_dirty.set_false();
    auto words = filtered_words();

    if (words.size() == 0)
        return;

    int const end = (int) words.size() - 1;
    if (ds != end)
    {
        ds += height - 3;  // 2 for borders, 1 for context

        if (ds > end)
            ds = end;

        // redraw but keep data
        // note that the filtered_words() call before will make the cache dirty again
        mark_dirty();
        cache_dirty.set_false();
    }
}


void AbstractListWindow::on_HOME()
{
    int & ds = display_start();
    if (ds != 0)
    {
        ds = 0;
        mark_dirty();
        cache_dirty.set_false();
    }
}


void AbstractListWindow::on_END()
{
    int & ds = display_start();

    cache_dirty.set_false();
    auto words = filtered_words();
    if (words.size() == 0)
        return;

    int const end = (int) words.size() - 1;
    if (ds != end)
    {
        ds = end;
        mark_dirty();
        cache_dirty.set_false();
    }
}


void AbstractListWindow::on_RETURN()
{
    int & ds = display_start();

    auto words = filtered_words();

    if (words.size() == 0)
        return;

    if (ds >= (int) words.size())
        ds = (int) words.size() - 1;

    std::string const & selected = string_from_index(words[ds]);

    // save old prefix to word_stack
    ws.push({cs.top().prefix, this, ds});

    // update completion stack
    while (cs.count() > 1)
        cs.pop();
    for (auto ch : selected)
        cs.push(ch);

    input_win.mark_dirty();

    on_post_RETURN();
}


std::vector<int> const & AbstractListWindow::filtered_words() const
{
    if (cache_dirty.is_dirty())
    {
        filtered_words_cache.clear();

        auto & c = cs.top();

        if (c.standard_completion.size() == 0)
            return filtered_words_cache;

        if (c.display_start >= (int) c.standard_completion.size())
            c.display_start = (int) c.standard_completion.size() - 1;

        auto const & unfiltered = unfiltered_words(c.standard_completion[c.display_start]);

        filtered_words_cache.reserve(unfiltered.size());
        for (auto word : unfiltered)
            if (wf.passes(word))
                filtered_words_cache.push_back(word);
    }

    return filtered_words_cache;
}


std::string const & AbstractListWindow::string_from_index(int index)
{
    return matchmaker::at(index);
}
