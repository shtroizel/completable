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



#include <chrono>
#include <iomanip>
#include <iostream>
#include <queue>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include <ncurses.h>

#include <matchmaker/matchmaker.h>



// Each time a letter is entered the new prefix and its completion is stored in the "completions" array,
// which is used as a stack. This means we should set MAX_COMPLETIONS to be larger than the longest word
// in the dictionary.
//
// 107 should be ok
static int const MAX_COMPLETIONS{107};

static int const PAGE_UP{339};
static int const PAGE_DOWN{338};
static int const TAB{9};
static int const HOME{262};
static int const END{360};

// support words of at least this many letters
static int const MIN_SUPPORTED_WORD_LENGTH{34};

// minimum required terminal height
static int const MIN_ROOT_Y{30};

// minimum required terminal width
static int const MIN_ROOT_X{80};


struct completion
{
    std::string prefix;                 // string typed so far
    int start{-1};                      // index of first word in dictionary starting with prefix
    int length{0};                      // number of words in the dictionary starting with prefix
    int display_start{-1};              // index of first word displayed in "Completion"
    int len_display_start{0};           // index of first word displayed in "Length Completion"
    std::vector<int> length_completion; // each "long index" is stored for length completion instead
                                        // of just start + length because they are noncontiguous
};


struct WIN
{
    WINDOW * w{nullptr};
    int height{0};
    int width{0};
    int y{0};
    int x{0};
    int root_y{0};
    int root_x{0};

    inline void new_window() { if (nullptr == w) w = newwin(height, width, y, x); }
    inline void update() { if (nullptr != w) { mvwin(w, y, x); wresize(w, height, width); } }
};


void grow(
    int ch,
    int & completion_count,
    completion * completions
);


void draw_input_win(
    WIN & win,
    int completion_count,
    completion * completions
);


void draw_complete_win(
    WIN & win,
    int completion_count,
    completion * completions
);


void draw_len_complete_win(
    WIN & win,
    int completion_count,
    completion * completions
);


void draw_pos_win(
    WIN & win,
    int completion_count,
    completion * completions
);


void draw_syn_win(
    WIN & win,
    int completion_count,
    completion * completions
);


void draw_ant_win(
    WIN & win,
    int completion_count,
    completion * completions
);


void shell();


void resize_complete_win(
    int root_y,
    int root_x,
    int target_complete_width,
    WIN & win
);


void resize_len_complete_win(
    int root_y,
    int root_x,
    WIN const & complete_win,
    WIN & win
);


void resize_input_win(
    int root_y,
    int root_x,
    int target_complete_width,
    WIN & win
);


void resize_pos_win(
    int root_y,
    int root_x,
    WIN & win
);


void resize_syn_or_ant_win(
    int root_y,
    int root_x,
    WIN const & neighbor_win,
    WIN & win
);



enum class Win{ Completion, LengthCompletion };
Win active_win = Win::Completion;


int main()
{
    initscr();
    noecho();
    curs_set(FALSE);

    // window dimensions
    int root_y{0};
    int root_x{0};
    getmaxyx(stdscr, root_y, root_x);
    int new_root_y{root_y};
    int new_root_x{root_x};

    // calculate width based on longest word in dictionary + 2 cols for borders
    int const target_complete_width{
        std::max(
            (int) matchmaker::at(matchmaker::from_longest(0)).size(),
            MIN_SUPPORTED_WORD_LENGTH
        )
        +
        2 // left and right borders
    };

    WIN input_win;
    resize_input_win(root_y, root_x, target_complete_width, input_win);
    input_win.new_window();
    keypad(input_win.w, true);

    WIN complete_win;
    resize_complete_win(root_y, root_x, target_complete_width, complete_win);
    complete_win.new_window();

    WIN len_complete_win;
    resize_len_complete_win(root_y, root_x, complete_win, len_complete_win);
    len_complete_win.new_window();

    WIN pos_win;
    resize_pos_win(root_y, root_x, pos_win);
    pos_win.new_window();

    WIN syn_win;
    resize_syn_or_ant_win(root_y, root_x, complete_win, syn_win);
    syn_win.new_window();

    WIN ant_win;
    resize_syn_or_ant_win(root_y, root_x, len_complete_win, ant_win);
    ant_win.new_window();


    completion completions[MAX_COMPLETIONS];
    completions[0].start = 0;
    completions[0].display_start = 0;
    completions[0].length = matchmaker::size();
    completions[0].len_display_start = 0;
    {
        std::priority_queue<int, std::vector<int>, std::greater<std::vector<int>::value_type>> q;
        for (int i = 0; i < matchmaker::size(); ++i)
            q.push(matchmaker::as_longest(i));
        while (!q.empty())
        {
            completions[0].length_completion.push_back(q.top());
            q.pop();
        }
    }

    int completion_count{1};

    int ch{0};

    while (1)
    {
        // terminal resized?
        getmaxyx(stdscr, new_root_y, new_root_x);
        if (new_root_y != root_y || new_root_x != root_x)
        {
            wclear(input_win.w);
            wclear(complete_win.w);
            wclear(len_complete_win.w);
            wclear(pos_win.w);
            wclear(syn_win.w);
            wclear(ant_win.w);

            wrefresh(input_win.w);
            wrefresh(complete_win.w);
            wrefresh(len_complete_win.w);
            wrefresh(pos_win.w);
            wrefresh(syn_win.w);
            wrefresh(ant_win.w);

            root_x = new_root_x;
            root_y = new_root_y;

            // recalculate all completions using new max_results
            for (int i = 0; i < completion_count; ++i)
            {
                matchmaker::complete(
                    completions[i].prefix,
                    completions[i].start,
                    completions[i].length
                );
            }

            resize_input_win(root_y, root_x, target_complete_width, input_win);
            resize_complete_win(root_y, root_x, target_complete_width, complete_win);
            resize_len_complete_win(root_y, root_x, complete_win, len_complete_win);
            resize_pos_win(root_y, root_x, pos_win);
            resize_syn_or_ant_win(root_y, root_x, complete_win, syn_win);
            resize_syn_or_ant_win(root_y, root_x, len_complete_win, ant_win);
        } // of terminal window resized

        draw_input_win(input_win, completion_count, completions);
        draw_complete_win(complete_win, completion_count, completions);
        draw_len_complete_win(len_complete_win, completion_count, completions);
        draw_pos_win(pos_win, completion_count, completions);
        draw_syn_win(syn_win, completion_count, completions);
        draw_ant_win(ant_win, completion_count, completions);

        ch = wgetch(input_win.w);

        if (ch > 31 && ch < 127)
        {
            grow(ch, completion_count, completions);
        }
        else if (ch == KEY_BACKSPACE && completion_count > 1)
        {
            --completion_count;
            completions[completion_count].prefix.clear();
            completions[completion_count].start = -1;
            completions[completion_count].display_start = -1;
            completions[completion_count].length = 0;
            completions[completion_count].len_display_start = 0;
            completions[completion_count].length_completion.clear();
        }
        else if (ch == TAB)
        {
            std::string const & prefix = completions[completion_count - 1].prefix;
            if (completions[completion_count - 1].length > 0)
            {
                std::string const & first_entry =
                        matchmaker::at(completions[completion_count - 1].start);

                auto const & cur_completion = completions[completion_count - 1];

                // find out the "target_completion_count" or the completion count after skipping
                // by common characters
                int target_completion_count = completion_count;
                bool ok = first_entry.size() > prefix.size();
                while (ok)
                {
                    for (
                        int i = cur_completion.start;
                        ok && i < cur_completion.start + cur_completion.length;
                        ++i
                    )
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

                // grow up to the target completion count
                for (int i = (int) prefix.size(); i < target_completion_count; ++i)
                    grow(first_entry[i], completion_count, completions);
            }
        }
        else if (ch == '$' || ch == '~' || ch == '`')
        {
            def_prog_mode();
            endwin();

            shell();

            reset_prog_mode();
            refresh();
        }
        else if (ch == KEY_LEFT)
        {
            active_win = Win::Completion;
        }
        else if (ch == KEY_RIGHT)
        {
            active_win = Win::LengthCompletion;
        }
        else if (ch == KEY_UP)
        {
            completion & c = completions[completion_count - 1];

            if (active_win == Win::Completion && c.display_start > c.start)
                --c.display_start;
            else if (active_win == Win::LengthCompletion && c.len_display_start > 0)
                --c.len_display_start;
        }
        else if (ch == KEY_DOWN)
        {
            completion & c = completions[completion_count - 1];

            if (active_win == Win::Completion && c.display_start < c.start + c.length - 1)
                ++c.display_start;
            else if (active_win == Win::LengthCompletion &&
                    c.len_display_start < (int) c.length_completion.size() - 1)
                ++c.len_display_start;
        }
        else if (ch == PAGE_UP)
        {
            completion & c = completions[completion_count - 1];

            if (active_win == Win::Completion)
            {
                c.display_start -= complete_win.height - 2;
                if (c.display_start < c.start)
                    c.display_start = c.start;
            }
            else if (active_win == Win::LengthCompletion)
            {
                c.len_display_start -= len_complete_win.height - 2;
                if (c.len_display_start < 0)
                    c.len_display_start = 0;
            }
        }
        else if (ch == PAGE_DOWN)
        {
            completion & c = completions[completion_count - 1];

            if (active_win == Win::Completion)
            {
                c.display_start += complete_win.height - 2;
                if (c.display_start >= c.start + c.length)
                    c.display_start = c.start + c.length - 1;
            }
            else if (active_win == Win::LengthCompletion)
            {
                c.len_display_start += len_complete_win.height - 2;
                if (c.len_display_start >= (int) c.length_completion.size())
                    c.len_display_start = (int) c.length_completion.size() - 1;
            }
        }
        else if (ch == HOME)
        {
            completion & c = completions[completion_count - 1];
            if (active_win == Win::Completion)
                c.display_start = c.start;
            else if (active_win == Win::LengthCompletion)
                c.len_display_start = 0;
        }
        else if (ch == END)
        {
            completion & c = completions[completion_count - 1];
            if (active_win == Win::Completion)
                c.display_start = c.start + c.length - 1;
            else if (active_win == Win::LengthCompletion)
                c.len_display_start = c.length_completion.size() - 1;
        }
    }

    delwin(complete_win.w);
    delwin(len_complete_win.w);
    delwin(pos_win.w);
    delwin(input_win.w);

    endwin();

    return 0;
}


void grow(
    int ch,
    int & completion_count,
    completion * completions
)
{
    if (completion_count >= MAX_COMPLETIONS) // should be impossible
        return;

    // start with previous prefix
    completions[completion_count].prefix = completions[completion_count - 1].prefix;

    // add new character
    completions[completion_count].prefix += ch;

    // get new completion
    matchmaker::complete(
        completions[completion_count].prefix,
        completions[completion_count].start,
        completions[completion_count].length
    );

    // calculate length completion
    std::priority_queue<int, std::vector<int>, std::greater<std::vector<int>::value_type>> q;
    for (
        int i = completions[completion_count].start;
        i < completions[completion_count].start + completions[completion_count].length;
        ++i
    )
        q.push(matchmaker::as_longest(i));
    while (!q.empty())
    {
        completions[completion_count].length_completion.push_back(q.top());
        q.pop();
    }

    // initialize display_start
    completions[completion_count].display_start = completions[completion_count].start;

    // initialize len_display_start
    completions[completion_count].len_display_start = 0;

    // update completion_count
    if (completions[completion_count].length > 0)
        ++completion_count;
}


void draw_input_win(
    WIN & win,
    int completion_count,
    completion * completions
)
{
    wclear(win.w);

    if (win.root_y < MIN_ROOT_Y || win.root_x < MIN_ROOT_X)
    {
        wrefresh(win.w);
        return;
    }

    box(win.w, 0, 0);

    // title
    std::string const title{"Input"};
    int const indent{win.width / 3 - (int) title.size() / 2};
    mvwaddch(win.w, 0, indent - 1, ' ');
    for (int i = 0; i < (int) title.size(); ++i)
        mvwaddch(win.w, 0, i + indent, title[i]);
    mvwaddch(win.w, 0, title.size() + indent, ' ');

    // draw prefix
    if (completion_count > 0)
    {
        std::string const & prefix = completions[completion_count - 1].prefix;
        for (int x = 0; x < win.width - 2 && x < (int) prefix.size(); ++x)
            mvwaddch(win.w, 1, x + 1, prefix[x]);
    }

    wrefresh(win.w);
}


void draw_complete_win(
    WIN & win,
    int completion_count,
    completion * completions
)
{
    wclear(win.w);

    if (win.root_y < MIN_ROOT_Y || win.root_x < MIN_ROOT_X)
    {
        wrefresh(win.w);
        return;
    }

    box(win.w, 0, 0);

    // title
    std::string const title{"Completion"};
    int const indent{win.width / 3 - (int) title.size() / 2};
    mvwaddch(win.w, 0, indent - 1, ' ');
    for (int i = 0; i < (int) title.size(); ++i)
        mvwaddch(win.w, 0, i + indent, title[i]);
    mvwaddch(win.w, 0, title.size() + indent, ' ');

    // completion
    completion & cur_completion = completions[completion_count - 1];
    int length = cur_completion.length - (cur_completion.display_start - cur_completion.start);
    for (int i = 0; i < length && i < win.height - 2; ++i)
    {
        std::string const & complete_entry = matchmaker::at(cur_completion.display_start + i);

        if (active_win == Win::Completion && i == 0)
            wattron(win.w, A_REVERSE);

        // draw complete_entry letter by letter
        for (int j = 0; j < (int) complete_entry.size() && j < win.width - 2; ++j)
        {
            mvwaddch(
                win.w,
                i + 1,
                j + 1,
                complete_entry[j]
            );
        }

        if (active_win == Win::Completion && i == 0)
            wattroff(win.w, A_REVERSE);
    }

    wrefresh(win.w);
}


void draw_len_complete_win(
    WIN & win,
    int completion_count,
    completion * completions
)
{
    wclear(win.w);

    if (win.root_y < MIN_ROOT_Y || win.root_x < MIN_ROOT_X)
    {
        wrefresh(win.w);
        return;
    }

    box(win.w, 0, 0);

    // title
    std::string const title{"Length Completion"};
    int const indent{win.width / 3 - (int) title.size() / 2};
    mvwaddch(win.w, 0, indent - 1, ' ');
    for (int i = 0; i < (int) title.size(); ++i)
        mvwaddch(win.w, 0, i + indent, title[i]);
    mvwaddch(win.w, 0, title.size() + indent, ' ');

    // completion
    completion & cur_completion = completions[completion_count - 1];
    int length = cur_completion.length_completion.size() - cur_completion.len_display_start;
    if (length < 0)
        return;

    int long_index{0};

    for (int i = 0; i < length && i < win.height - 2; ++i)
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

        if (active_win == Win::LengthCompletion && i == 0)
            wattron(win.w, A_REVERSE);

        // draw entry
        for (int j = 0; j < (int) complete_entry.size() && j < win.width - 2; ++j)
        {
            mvwaddch(
                win.w,
                i + 1,
                j + 1,
                complete_entry[j]
            );
        }

        if (active_win == Win::LengthCompletion && i == 0)
            wattroff(win.w, A_REVERSE);
    }

    wrefresh(win.w);
}


void draw_pos_win(
    WIN & win,
    int completion_count,
    completion * completions
)
{
    wclear(win.w);

    if (win.root_y < MIN_ROOT_Y || win.root_x < MIN_ROOT_X)
    {
        wrefresh(win.w);
        return;
    }

    box(win.w, 0, 0);

    int const cell_width{16};

    // title
    std::string const title{"Parts of Speech"};
    int const indent{win.width / 3 - (int) title.size() / 2};
    mvwaddch(win.w, 0, indent - 1, ' ');
    for (int i = 0; i < (int) title.size(); ++i)
        mvwaddch(win.w, 0, i + indent, title[i]);
    mvwaddch(win.w, 0, title.size() + indent, ' ');

    // parts of speech
    int selected{0};
    if (active_win == Win::Completion)
    {
        selected = completions[completion_count - 1].display_start;
    }
    else if (active_win == Win::LengthCompletion)
    {
        int length_completion_index = completions[completion_count - 1].len_display_start;
        if (length_completion_index >= (int) completions[completion_count - 1].length_completion.size())
            return;
        if (length_completion_index < 0)
            return;
        int long_index = completions[completion_count - 1].length_completion[length_completion_index];
        selected = matchmaker::from_longest(long_index);
    }
    else
    {
        return;
    }

    auto const & flagged_pos = matchmaker::flagged_parts_of_speech(selected);

    int x = 1;
    int i = 0;
    int y = 1;
    while (i < (int) matchmaker::all_parts_of_speech().size())
    {
        if (flagged_pos[i] != 0)
            wattron(win.w, A_REVERSE);

        for(
            int j = 0;
            j < (int) matchmaker::all_parts_of_speech()[i].size() && j < x + cell_width - 2;
            ++j
        )
            mvwaddch(win.w, y, j + x, matchmaker::all_parts_of_speech()[i][j]);

        if (flagged_pos[i] != 0)
            wattroff(win.w, A_REVERSE);

        ++i;

        x += cell_width;

        if (x > cell_width * 5)
        {
            if (y < 4)
            {
                y += 1;
                x = 1;
            }
            else
            {
                break;
            }
        }
    }

    wrefresh(win.w);
}


void draw_syn_win(
    WIN & win,
    int completion_count,
    completion * completions
)
{
    wclear(win.w);

    if (win.root_y < MIN_ROOT_Y || win.root_x < MIN_ROOT_X)
    {
        wrefresh(win.w);
        return;
    }

    box(win.w, 0, 0);

    // title
    std::string const title{"Synonyms"};
    int const indent{win.width / 3 - (int) title.size() / 2};
    mvwaddch(win.w, 0, indent - 1, ' ');
    for (int i = 0; i < (int) title.size(); ++i)
        mvwaddch(win.w, 0, i + indent, title[i]);
    mvwaddch(win.w, 0, title.size() + indent, ' ');

    // currently selected word
    int selected{0};
    if (active_win == Win::Completion)
    {
        selected = completions[completion_count - 1].display_start;
    }
    else if (active_win == Win::LengthCompletion)
    {
        int length_completion_index = completions[completion_count - 1].len_display_start;
        if (length_completion_index >= (int) completions[completion_count - 1].length_completion.size())
            return;
        if (length_completion_index < 0)
            return;
        int long_index = completions[completion_count - 1].length_completion[length_completion_index];
        selected = matchmaker::from_longest(long_index);
    }
    else
    {
        return;
    }

    // synonyms
    auto const & synonyms = matchmaker::synonyms(selected);

    for (int i = 0; i < (int) synonyms.size() && i < win.height - 2; ++i)
    {
        std::string const & syn = matchmaker::at(synonyms[i]);

        // draw synonym
        for (int j = 0; j < (int) syn.length() && j < win.width - 2; ++j)
        {
            mvwaddch(
                win.w,
                i + 1,
                j + 1,
                syn[j]
            );
        }

    }

    wrefresh(win.w);
}


void draw_ant_win(
    WIN & win,
    int completion_count,
    completion * completions
)
{
    wclear(win.w);

    if (win.root_y < MIN_ROOT_Y || win.root_x < MIN_ROOT_X)
    {
        wrefresh(win.w);
        return;
    }

    box(win.w, 0, 0);

    // title
    std::string const title{"Antonyms"};
    int const indent{win.width / 3 - (int) title.size() / 2};
    mvwaddch(win.w, 0, indent - 1, ' ');
    for (int i = 0; i < (int) title.size(); ++i)
        mvwaddch(win.w, 0, i + indent, title[i]);
    mvwaddch(win.w, 0, title.size() + indent, ' ');

    // currently selected word
    int selected{0};
    if (active_win == Win::Completion)
    {
        selected = completions[completion_count - 1].display_start;
    }
    else if (active_win == Win::LengthCompletion)
    {
        int length_completion_index = completions[completion_count - 1].len_display_start;
        if (length_completion_index >= (int) completions[completion_count - 1].length_completion.size())
            return;
        if (length_completion_index < 0)
            return;
        int long_index = completions[completion_count - 1].length_completion[length_completion_index];
        selected = matchmaker::from_longest(long_index);
    }
    else
    {
        return;
    }

    // antonyms
    auto const & antonyms = matchmaker::antonyms(selected);

    for (int i = 0; i < (int) antonyms.size() && i < win.height - 2; ++i)
    {
        std::string const & ant = matchmaker::at(antonyms[i]);

        // draw synonym
        for (int j = 0; j < (int) ant.length() && j < win.width - 2; ++j)
        {
            mvwaddch(
                win.w,
                i + 1,
                j + 1,
                ant[j]
            );
        }

    }

    wrefresh(win.w);
}


void shell()
{
    std::string const DELIMITER{" "};
    int index{-1};
    bool found{false};
    bool help{false};
    std::vector<std::string const *> pos;
    int const MAX_INDEX_DIGITS =
        []()
        {
            int digit_count{0};
            for (int i = 1; i < matchmaker::size(); i *= 10)
                ++digit_count;

            return digit_count;
        }();

    while (true)
    {
        std::cout << "\n\n";
        if (help)
            std::cout << "{ just enter a word for lookup                                                }\n"
                      << "{ prefix the word with ':' for completion                                     }\n"
                      << "{ use  :it <index> <count>        to iterate <count> words from <index>       }\n"
                      << "{ use  :pos <word>                for parts of speech of <word>               }\n"
                      << "{ use  :s <word>                  for synonyms of <word>                      }\n"
                      << "{ use  :a <word>                  for antonyms of <word>                      }\n"
                      << "{ use  :lon <index> <count>       like ':it' but uses 'by_longest()'          }\n"
                      << "{ use  :len                       to list ':lon' indexes by length            }\n"
                      ;

        std::cout <<     "{ use  :help                      to toggle help                              }\n"
                  <<     "matchmaker (" << matchmaker::size() << ") $  ";

        std::string line;
        std::getline(std::cin, line);
        if (std::cin.eof() == 1)
        {
            std::cout << std::endl;
            break;
        }
        std::vector<std::string> words;
        std::stringstream ss(line);
        std::string token;
        while (std::getline(ss, token, ' '))
            words.push_back(token);

        if (words.size() == 0)
        {
            std::cout << std::endl;
            break;
        }
        else if (words.size() == 1 && words[0].size() > 0)
        {
            if (words[0] == ":len")
            {
                std::cout << "The following index offsets are for :longest ('<nth>' values)" << std::endl;
                std::cout << "The later index is inclusive\n" << std::endl;

                int index{0};
                int count{0};
                for (auto l : matchmaker::lengths())
                {
                    if (matchmaker::length_location(l, index, count))
                        std::cout << "    " << std::setw(MAX_INDEX_DIGITS) << l
                                  << " letter words begin at index [" << std::setw(MAX_INDEX_DIGITS)
                                  << index << "] with a count of: " << std::to_string(count) << std::endl;
                    else
                        std::cout << "OOPS!" << std::endl; // should be impossible
                }
            }
            else if (words[0] == ":help")
            {
                help = !help;
            }
            else if (words[0][0] == ':')
            {
                completion c;
                c.prefix = words[0].substr(1);
                auto start = std::chrono::high_resolution_clock::now();
                matchmaker::complete(c.prefix, c.start, c.length);
                auto stop = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                std::cout << "completion (" << c.length << ") : ";
                for (int i = c.start; i < c.start + c.length; ++i)
                    std::cout << " " << matchmaker::at(i);
                std::cout << "\ncompletion done in " << duration.count() << " microseconds" << std::endl;
            }
            else
            {
                std::cout << "       [";
                auto start = std::chrono::high_resolution_clock::now();
                index = matchmaker::lookup(words[0], &found);
                auto stop = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                std::cout << index << "], longest[" << matchmaker::as_longest(index) << "] :  '"
                        << matchmaker::at(index) << "' ";
                if (!found)
                    std::cout << "(index if existed) ";
                std::cout << "       lookup time: " << duration.count() << " microseconds" << std::endl;
            }
        }
        else if (words.size() == 2)
        {
            if (words[0] == ":s")
            {
                std::cout << "       [";
                auto start = std::chrono::high_resolution_clock::now();
                index = matchmaker::lookup(words[1], &found);
                auto synonyms = matchmaker::synonyms(index);
                auto stop = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                std::cout << index << "] :  '" << matchmaker::at(index) << "' ";
                if (!found)
                    std::cout << "(index if existed) ";
                std::cout << " syn:  ";
                for (int j = 0; j < (int) synonyms.size() - 1; ++j)
                    std::cout << matchmaker::at(synonyms[j]) << ", ";
                if (synonyms.size() > 0)
                    std::cout << matchmaker::at(synonyms[synonyms.size() - 1]);
                else
                    std::cout << " NONE AVAILABLE";
                std::cout << "\n       -------> lookup + synonym retrieval time: "
                          << duration.count() << " microseconds" << std::endl;
            }
            else if (words[0] == ":a")
            {
                std::cout << "       [";
                auto start = std::chrono::high_resolution_clock::now();
                index = matchmaker::lookup(words[1], &found);
                auto antonyms = matchmaker::antonyms(index);
                auto stop = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                std::cout << index << "] :  '" << matchmaker::at(index) << "' ";
                if (!found)
                    std::cout << "(index if existed) ";
                std::cout << " ant:  ";
                for (int j = 0; j < (int) antonyms.size() - 1; ++j)
                    std::cout << matchmaker::at(antonyms[j]) << ", ";
                if (antonyms.size() > 0)
                    std::cout << matchmaker::at(antonyms[antonyms.size() - 1]);
                else
                    std::cout << " NONE AVAILABLE";
                std::cout << "\n       -------> lookup + antonym retrieval time: "
                          << duration.count() << " microseconds" << std::endl;
            }
            else if (words[0] == ":pos")
            {
                for (int i = 1; i < (int) words.size(); ++i)
                {
                    std::cout << "       [";
                    index = matchmaker::lookup(words[i], &found);
                    auto start = std::chrono::high_resolution_clock::now();
                    matchmaker::parts_of_speech(index, pos);
                    auto stop = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                    std::cout << index << "] :  '" << matchmaker::at(index) << "' ";
                    if (!found)
                        std::cout << "(index if existed) ";
                    std::cout << " pos:  ";
                    for (int j = 0; j < (int) pos.size() - 1; ++j)
                        std::cout << *pos[j] << ", ";

                    if (pos.size() > 0)
                        std::cout << *pos[pos.size() - 1];
                    else
                        std::cout << "NONE AVAILABLE";

                    std::cout << "\n       -------> parts_of_speech() time: "
                              << duration.count() << " microseconds" << std::endl;
                }
            }
        }
        else if (words.size() == 3)
        {
            int start{0}; try { start = std::stoi(words[1]); } catch (...) { continue; }
            int count{0}; try { count = std::stoi(words[2]); } catch (...) { continue; }

            if (words[0] == ":lon")
            {
                for (int i = start; i < (int) matchmaker::by_longest().size() && i < start + count; ++i)
                    std::cout << "       [" << std::setw(MAX_INDEX_DIGITS) << matchmaker::by_longest()[i]
                              << "], longest[" << std::setw(MAX_INDEX_DIGITS) << i << "]  "
                              << matchmaker::at(matchmaker::by_longest()[i]) << " has "
                              << matchmaker::at(matchmaker::by_longest()[i]).size()
                              << " characters" << std::endl;
            }
            else if (words[0] == ":it")
            {

                for (int i = start; i < matchmaker::size() && i < start + count; ++i)
                {
                    auto start = std::chrono::high_resolution_clock::now();
                    std::string const & str = matchmaker::at(i);
                    auto stop = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                    std::cout << "       [" << std::setw(MAX_INDEX_DIGITS) << i << "], longest["
                              << std::setw(MAX_INDEX_DIGITS) << matchmaker::as_longest(i) << "] :  '"
                              << str << "' accessed in " << duration.count() << " microseconds\n";
                }
                std::cout << std::flush;
            }
        }
    }
}


void resize_complete_win(int root_y, int root_x, int target_complete_width, WIN & win)
{
    win.root_y = root_y;
    win.root_x = root_x;

    win.y = 3;
    win.x = 0;

    // height
    int combined_height = root_y - 5 - win.y;
    win.height = win.y + ((combined_height * 9) / 17);

    // width
    if (target_complete_width < root_x)
        win.width = target_complete_width;
    else
        win.width = root_x;

    win.update();
}


void resize_len_complete_win(int root_y, int root_x, WIN const & complete_win, WIN & win)
{
    win.root_y = root_y;
    win.root_x = root_x;

    win.y = complete_win.y + complete_win.height;
    win.x = 0;
    win.height = root_y - win.y - 5;
    win.width = complete_win.width;

    win.update();
}


void resize_input_win(int root_y, int root_x, int target_complete_width, WIN & win)
{
    win.root_y = root_y;
    win.root_x = root_x;

    win.height = 3;
    if (target_complete_width < root_x)
        win.width = target_complete_width;
    else
        win.width = root_x;
    win.y = 0;
    win.x = std::min(root_x, 80) / 2 - win.width / 2;

    win.update();
}


void resize_pos_win(int root_y, int root_x, WIN & win)
{
    win.root_y = root_y;
    win.root_x = root_x;

    win.height = 5;
    win.width = std::min(root_x, 80);
    win.y = root_y - win.height;
    win.x = 0;

    win.update();
}


void resize_syn_or_ant_win(
    int root_y,
    int root_x,
    WIN const & neighbor_win,
    WIN & win
)
{
    win.root_y = root_y;
    win.root_x = root_x;

    win.height = neighbor_win.height;
    win.width = neighbor_win.width;
    win.y = neighbor_win.y;
    win.x = MIN_ROOT_X - win.width;
    win.update();
}
