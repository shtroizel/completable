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
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include <ncurses.h>

#include <matchmaker/matchmaker.h>



// Each time a letter is entered the new prefix and its completion is stored in the "completions" array,
// which is used as a stack. This means we should set MAX_COMPLETIONS to be longer than the longest word
// in the dictionary.
//
// longest word so far is 45 letters so 107 should be ok
static int const MAX_COMPLETIONS{107};

static int const PAGE_UP{339};
static int const PAGE_DOWN{338};
static int const TAB{9};
static int const HOME{262};
static int const END{360};


struct completion
{
    std::string prefix;      // string typed so far
    int start{-1};           // index of first word in dictionary starting with prefix
    int length{0};           // number of words in the dictionary starting with prefix
    int display_start{-1};   // index of first word displayed starting with prefix
};


void grow(
    int ch,
    int & completion_count,
    completion * completions
);


void draw_complete_win(
    int height,
    int width,
    int completion_count,
    completion * completions,
    WINDOW * win
);


void draw_pos_win(
    int height,
    int width,
    int completion_count,
    completion * completions,
    WINDOW * win
);


void shell();


int main()
{
    initscr();
    noecho();
    curs_set(FALSE);
    halfdelay(5);

    // window dimensions
    int root_y{0};
    int root_x{0};
    getmaxyx(stdscr, root_y, root_x);
    int new_root_y{root_y};
    int new_root_x{root_x};

    // calculate width based on longest word in dictionary
    int target_complete_width{0};
    for (int i = 0; i < matchmaker::size(); ++i)
        if ((int) matchmaker::at(i).size() > target_complete_width)
            target_complete_width = matchmaker::at(i).size();
    target_complete_width += 2;
    if (target_complete_width < 33)
        target_complete_width = 33;
    int complete_width{target_complete_width};

    int complete_height{root_y};

    WINDOW * complete_win = newwin(complete_height, complete_width, 0, 0);
    keypad(complete_win, true);

    int const target_pos_height{19};
    int pos_height{target_pos_height};
    int const target_pos_width{23};
    int pos_width{target_pos_width};
    WINDOW * pos_win = newwin(pos_height, pos_width, 0, complete_width + 1);
    keypad(pos_win, true);

    completion completions[MAX_COMPLETIONS];
    completions[0].start = 0;
    completions[0].display_start = 0;
    completions[0].length = matchmaker::size();
    int completion_count{1};

    int ch{0};

    while (1)
    {
        // terminal resized?
        getmaxyx(stdscr, new_root_y, new_root_x);
        if (new_root_y != root_y || new_root_x != root_x)
        {
            root_x = new_root_x;
            root_y = new_root_y;

            wclear(complete_win);
            complete_width = target_complete_width;
            if (complete_width > root_x)
                complete_width = root_x;
            complete_height = root_y;
            wresize(complete_win, complete_height, complete_width);

            // recalculate all completions using new max_results
            for (int i = 0; i < completion_count; ++i)
            {
                matchmaker::complete(
                    completions[i].prefix,
                    completions[i].start,
                    completions[i].length
                );
            }


            wclear(pos_win);

            pos_height = target_pos_height;
            if (pos_height > root_y)
                pos_height = root_y;

            pos_width = target_pos_width;
            int width_available = root_x - complete_width - 1;
            if (pos_width > width_available)
                pos_width = width_available;

            wresize(pos_win, pos_height, pos_width);

            wrefresh(complete_win);
            wrefresh(pos_win);
        }

        draw_complete_win(complete_height, complete_width, completion_count, completions, complete_win);
        draw_pos_win(pos_height, pos_width, completion_count, completions, pos_win);

        ch = wgetch(complete_win);

        if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
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
        else if (ch == KEY_UP)
        {
            completion & c = completions[completion_count - 1];
            if (c.display_start > c.start)
                --c.display_start;
        }
        else if (ch == KEY_DOWN)
        {
            completion & c = completions[completion_count - 1];
            if (c.display_start < c.start + c.length - 1)
                ++c.display_start;
        }
        else if (ch == PAGE_UP)
        {
            completion & c = completions[completion_count - 1];
            c.display_start -= complete_height - 4;
            if (c.display_start < c.start)
                c.display_start = c.start;
        }
        else if (ch == PAGE_DOWN)
        {
            completion & c = completions[completion_count - 1];
            c.display_start += complete_height - 4;
            if (c.display_start >= c.start + c.length)
                c.display_start = c.start + c.length - 1;
        }
        else if (ch == HOME)
        {
            completion & c = completions[completion_count - 1];
            c.display_start = c.start;
        }
        else if (ch == END)
        {
            completion & c = completions[completion_count - 1];
            c.display_start = c.start + c.length - 1;
        }
    }

    delwin(complete_win);
    endwin();

    return 0;
}


void grow(
    int ch,
    int & completion_count,
    completion * completions
)
{
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

    // initialize display_start
    completions[completion_count].display_start = completions[completion_count].start;

    // update completion_count
    if (completions[completion_count].length > 0 && completion_count < MAX_COMPLETIONS)
        ++completion_count;
}


void draw_complete_win(
    int height,
    int width,
    int completion_count,
    completion * completions,
    WINDOW * win
)
{
    wclear(win);

    if (height < 5)
        return;

    if (width < 3)
        return;

    box(win, 0, 0);

    // draw prefix
    if (completion_count > 0)
    {
        std::string const & prefix = completions[completion_count - 1].prefix;
        for (int x = 0; x < width - 2 && x < (int) prefix.size(); ++x)
            mvwaddch(win, 1, x + 1, prefix[x]);
    }

    // separator
    mvwaddch(win, 2, 0, ACS_LTEE);
    for (int i = 1; i < width - 1; ++i)
        mvwaddch(win, 2, i, ACS_HLINE);
    mvwaddch(win, 2, width - 1, ACS_RTEE);

    // completion
    completion & cur_completion = completions[completion_count - 1];
    int length = cur_completion.length - (cur_completion.display_start - cur_completion.start);
    for (int i = 0; i < length && i < height - 4; ++i)
    {
        std::string const & complete_entry = matchmaker::at(cur_completion.display_start + i);

        if (i == 0)
            wattron(win, A_REVERSE);

        // draw complete_entry letter by letter
        for (int j = 0; j < (int) complete_entry.size() && j < width - 2; ++j)
        {
            mvwaddch(
                win,
                i + 3,
                j + 1,
                complete_entry[j]
            );
        }

        if (i == 0)
            wattroff(win, A_REVERSE);
    }

    wrefresh(win);
}


void draw_pos_win(
    int height,
    int width,
    int completion_count,
    completion * completions,
    WINDOW * win
)
{
    wclear(win);

    if (height < 5)
        return;

    if (width < 3)
        return;

    box(win, 0, 0);

    // title
    static std::string const pos_title{"Parts of Speech"};

    for (int i = 0; i < (int) pos_title.size() && i < width - 2; ++i)
        mvwaddch(win, 1, i + 1, pos_title[i]);

    // separator
    mvwaddch(win, 2, 0, ACS_LTEE);
    for (int i = 1; i < width - 1; ++i)
        mvwaddch(win, 2, i, ACS_HLINE);
    mvwaddch(win, 2, width - 1, ACS_RTEE);

    // completion
    int selected = completions[completion_count - 1].display_start;
    std::vector<std::string const *> pos_vect;
    matchmaker::parts_of_speech(selected, pos_vect);

    static std::array<std::pair<std::string, bool>, 15> pos = {
        std::make_pair("Noun", false ),
        std::make_pair("Plural", false),
        std::make_pair("Noun Phrase", false),
        std::make_pair("Verb (usu participle)", false),
        std::make_pair("Verb (transitive)", false),
        std::make_pair("Verb (intransitive)", false),
        std::make_pair("Adjective", false),
        std::make_pair("Adverb", false),
        std::make_pair("Conjunction", false),
        std::make_pair("Preposition", false),
        std::make_pair("Interjection", false),
        std::make_pair("Pronoun", false),
        std::make_pair("Definite Article", false),
        std::make_pair("Indefinite Article", false),
        std::make_pair("Nominative", false)
    };

    for (auto & p : pos)
        p.second = false;

    for (auto pv : pos_vect)
        for (auto & p : pos)
            if (*pv == p.first)
                p.second = true;

    for (int i = 0; i < (int) pos.size() && i < height - 4; ++i)
    {
        if (pos[i].second)
            wattron(win, A_REVERSE);

        for (int j = 0; j < (int) pos[i].first.size() && j < width - 2; ++j)
            mvwaddch(win, i + 3, j + 1, pos[i].first[j]);

        if (pos[i].second)
            wattroff(win, A_REVERSE);
    }

    wrefresh(win);
}


void shell()
{
    int index{-1};
    bool found{false};

    while (true)
    {
        std::cout << "matchmaker (" << matchmaker::size()
                    << ")  { prefix with ':' for completion } { use :abc for all } $  ";

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
        else if (words.size() == 1)
        {
            if (words[0] == ":abc")
            {
                for (int i = 0; i < matchmaker::size(); ++i)
                {
                    auto start = std::chrono::high_resolution_clock::now();
                    std::string const & str = matchmaker::at(i);
                    auto stop = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                        stop - start
                    );
                    std::cout << "       [" << i << "] :  '" << str << "' accessed in "
                                << duration.count() << " microseconds\n";
                }
                std::cout << std::flush;
                continue;
            }
            else if (words[0].size() && words[0][0] == ':')
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
                std::cout << "\ncompletion done in " << duration.count()
                            << " microseconds" << std::endl;
                continue;
            }
        }

        for (auto const & word : words)
        {
            std::cout << "       [";
            auto start = std::chrono::high_resolution_clock::now();
            index = matchmaker::lookup(word, &found);
            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
            std::cout << index << "] :  '" << matchmaker::at(index) << "'     lookup time: "
                    << duration.count() << " microseconds" << std::endl;
        }
        std::cout << std::endl;
    }
}
