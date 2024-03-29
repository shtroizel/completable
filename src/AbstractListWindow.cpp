#include "AbstractListWindow.h"

#include <ncurses.h>

#include "AbstractTab.h"
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
        int word_len{0};
        char const * word = string_from_index(words[display_start() + i], &word_len);

        // highlight first word
        if (is_active() && i == 0)
            wattron(w, A_REVERSE);

        int j = 0;
        for (; j < word_len && j < width - 2; ++j) // 2 for borders (left, right)
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

    int selected_len{0};
    char const * selected = string_from_index(words[ds], &selected_len);

    // save old prefix to word_stack
    ws.push({cs.top().prefix, this, ds});

    // update completion stack
    while (cs.count() > 1)
        cs.pop();
    for (int i = 0; i < selected_len; ++i)
        cs.push(selected[i]);

    input_win.mark_dirty();

    on_post_RETURN();
}


void AbstractListWindow::on_DELETE()
{
    if (ws.size() == 0)
        return;

    if (!belongs_to_active_tab())
        return;

    auto & [s, w, ds] = ws.top();

    while (cs.count() > 1)
        cs.pop();

    for (auto ch : s)
        cs.push(ch);

    ws.pop();

    auto active_tab = AbstractTab::get_active_tab().as_AbstractTab();
    active_tab->set_active_window(w);
    input_win.mark_dirty();
    display_start() = ds;
}


void AbstractListWindow::on_TAB()
{
    auto const & c = cs.top();
    int prefix_len = (int) c.prefix.length();

    if (c.standard_completion.size() > 0)
    {
        int first_entry_len{0};
        char const * first_entry = matchmaker::at(c.standard_completion[0], &first_entry_len);

        // find out the "target_completion_count" or the completion count after skipping
        // by common characters
        int target_completion_count = cs.count() - 1;
        bool ok = first_entry_len > prefix_len;
        while (ok)
        {
            for (auto i : c.standard_completion)
            {
                int entry_len{0};
                char const * entry = matchmaker::at(i, &entry_len);

                // are entry and first_entry long enough to compare at target_completion_count?
                if (entry_len <= target_completion_count)
                    ok = false;
                else if (first_entry_len <= target_completion_count)
                    ok = false;

                else if (entry[target_completion_count] != first_entry[target_completion_count])
                    ok = false;
            }

            if (ok)
                ++target_completion_count;
        }

        bool cs_modified{false};
        // grow up to the target completion count
        for (int i = prefix_len; i < target_completion_count && i < first_entry_len; ++i)
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

        int const * unfiltered{nullptr};
        int unfiltered_count{0};
        unfiltered_words(c.standard_completion[c.display_start], &unfiltered, &unfiltered_count);

        words_cache.reserve(unfiltered_count);

        if (!apply_filter())
            words_cache.assign(unfiltered, unfiltered + unfiltered_count);
        else
            for (int i = 0; i < unfiltered_count; ++i)
                if (wf.passes(unfiltered[i]))
                    words_cache.push_back(unfiltered[i]);

    }

    return words_cache;
}


char const * AbstractListWindow::string_from_index(int index, int * len)
{
    return matchmaker::at(index, len);
}
