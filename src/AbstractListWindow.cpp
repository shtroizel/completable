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

#include "AbstractPage.h"
#include "CompletionStack.h"
#include "InputWindow.h"
#include "Layer.h"
#include "matchmaker.h"
#include "word_filter.h"



AbstractListWindow::AbstractListWindow(
    CompletionStack & cs,
    WordStack & ws,
    InputWindow & iw,
    word_filter & f
)
    : AbstractCompletionDataWindow(cs, ws)
    , input_win(iw)
    , wf(f)
{
    iw.add_dirty_dependency(this);
}


void AbstractListWindow::draw_hook()
{
    auto words = get_words();

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


Layer::Type AbstractListWindow::layer() const
{
    return Layer::Bottom::grab();
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
    auto words = get_words();
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
    auto words = get_words();

    if (words.size() == 0)
        return;

    int const end = (int) words.size() - 1;
    if (ds != end)
    {
        ds += height - 3;  // 2 for borders, 1 for context

        if (ds > end)
            ds = end;

        // redraw but keep data
        // note that the get_words() call before will make the cache dirty again
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
    auto words = get_words();
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

    auto words = get_words();

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


void AbstractListWindow::on_DELETE()
{
    if (ws.size() == 0)
        return;

    if (!belongs_to_active_page())
        return;

    auto & [s, w, ds] = ws.top();

    while (cs.count() > 1)
        cs.pop();

    for (auto ch : s)
        cs.push(ch);

    ws.pop();

    auto active_page = AbstractPage::get_active_page();
    active_page->set_active_window(w);
    input_win.mark_dirty();
    display_start() = ds;
}


void AbstractListWindow::on_TAB()
{
    auto const & c = cs.top();
    std::string const & prefix = c.prefix;

    if (c.standard_completion.size() > 0)
    {
        std::string const & first_entry = matchmaker::at(c.standard_completion[0]);

        // find out the "target_completion_count" or the completion count after skipping
        // by common characters
        int target_completion_count = cs.count() - 1;
        bool ok = first_entry.size() > prefix.size();
        while (ok)
        {
            for (auto i : c.standard_completion)
            {
                std::string const & entry = matchmaker::at(i);

                if ((int) entry.size() < target_completion_count)
                    ok = false;
                else if ((int) first_entry.size() < target_completion_count)
                    ok = false;
                else if (entry[target_completion_count] != first_entry[target_completion_count])
                    ok = false;
            }

            if (ok)
                ++target_completion_count;
        }

        bool cs_modified{false};
        // grow up to the target completion count
        for (int i = (int) prefix.size(); i < target_completion_count; ++i)
        {
            cs.push(first_entry[i]);
            cs_modified = true;
        }

        if (cs_modified)
            input_win.mark_dirty();
    }
}


void AbstractListWindow::on_BACKSPACE()
{
    int old_count = cs.count();
    cs.pop();
    int new_count = cs.count();
    if (new_count != old_count)
        input_win.mark_dirty();
}


void AbstractListWindow::on_printable_ascii(int key)
{
    bool old_count = cs.count();
    cs.push(key);
    if (cs.count() != old_count)
        input_win.mark_dirty();
}


std::vector<int> const & AbstractListWindow::get_words() const
{
    if (cache_dirty.is_dirty())
    {
        words_cache.clear();

        auto & c = cs.top();

        if (c.standard_completion.size() == 0)
            return words_cache;

        if (c.display_start >= (int) c.standard_completion.size())
            c.display_start = (int) c.standard_completion.size() - 1;

        auto const & unfiltered = unfiltered_words(c.standard_completion[c.display_start]);

        words_cache.reserve(unfiltered.size());

        if (!apply_filter())
            words_cache = unfiltered;
        else
            for (auto word : unfiltered)
                if (wf.passes(word))
                    words_cache.push_back(word);

    }

    return words_cache;
}


std::string const & AbstractListWindow::string_from_index(int index)
{
    return matchmaker::at(index);
}
