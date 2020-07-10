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


void grow(
    int ch,
    int max_results,
    int & completion_count,
    std::pair<std::string, std::vector<int>> * completions
);


void draw_complete_win(
    int height,
    int width,
    int completion_count,
    std::pair<std::string, std::vector<int>> * completions,
    WINDOW * win
);




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
    int complete_width{0};
    for (int i = 0; i < matchmaker::size(); ++i)
        if ((int) matchmaker::at(i).size() > complete_width)
            complete_width = matchmaker::at(i).size();
    complete_width += 2;

    int complete_height{root_y};

    WINDOW * complete_win = newwin(complete_height, complete_width, 0, 0);
    keypad(complete_win, true);

    //             prefix, matchmaker indexes forming the completion list
    std::pair<std::string, std::vector<int>> completions[MAX_COMPLETIONS];
    int completion_count{0};

    int max_results{complete_height - 4};

    draw_complete_win(complete_height, complete_width, completion_count, completions, complete_win);

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
            wrefresh(complete_win);

            complete_height = root_y;
            max_results = complete_height - 4;
            wresize(complete_win, complete_height, complete_width);

            // recalculate all completions using new max_results
            for (int i = 0; i < completion_count; ++i)
            {
                matchmaker::complete(
                    completions[i].first,
                    max_results,
                    completions[i].second
                );
            }
        }

        draw_complete_win(complete_height, complete_width, completion_count, completions, complete_win);

        ch = wgetch(complete_win);

        if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
        {
            grow(ch, max_results, completion_count, completions);
        }
        else if (ch == KEY_BACKSPACE && completion_count > 0)
        {
            --completion_count;
            completions[completion_count].first.clear();
            completions[completion_count].second.clear();
        }
        else if (ch == 9) // TAB
        {
            if (completion_count > 0)
            {
                std::string const & prefix = completions[completion_count - 1].first;
                if (completions[completion_count - 1].second.size() > 0)
                {
                    std::string const & first_entry =
                            matchmaker::at(completions[completion_count - 1].second.at(0));

                    auto const & cur_completion = completions[completion_count - 1].second;

                    // find out the "target_completion_count" or the completion count after skipping
                    // by common characters
                    int target_completion_count = completion_count;
                    bool ok = first_entry.size() > prefix.size();
                    while (ok)
                    {
                        for (int i = 0; ok && i < (int) cur_completion.size(); ++i)
                        {
                            std::string const & entry =
                                    matchmaker::at(completions[completion_count - 1].second.at(i));

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
                        grow(first_entry[i], max_results, completion_count, completions);
                }
            }
        }
    }

    delwin(complete_win);
    endwin();

    return 0;
}


void grow(
    int ch,
    int max_results,
    int & completion_count,
    std::pair<std::string, std::vector<int>> * completions
)
{
    // start with previous prefix
    if (completion_count > 0)
        completions[completion_count].first = completions[completion_count - 1].first;
    else
        completions[completion_count].first.clear();

    // add new character to prefix
    completions[completion_count].first += ch;

    // get new completion
    matchmaker::complete(
        completions[completion_count].first,
        max_results,
        completions[completion_count].second
    );

    // update completion_count
    if (completions[completion_count].second.size() > 0 && completion_count < MAX_COMPLETIONS)
        ++completion_count;
}


void draw_complete_win(
    int height,
    int width,
    int completion_count,
    std::pair<std::string, std::vector<int>> * completions,
    WINDOW * win
)
{
    wclear(win);

    box(win, 0, 0);

    // draw prefix
    if (completion_count > 0)
    {
        std::string const & prefix = completions[completion_count - 1].first;
        for (int x = 0; x < width - 2 && x < (int) prefix.size(); ++x)
            mvwaddch(win, 1, x + 1, prefix[x]);
    }

    // separator
    mvwaddch(win, 2, 0, ACS_LTEE);
    for (int i = 1; i < width - 1; ++i)
        mvwaddch(win, 2, i, ACS_HLINE);
    mvwaddch(win, 2, width - 1, ACS_RTEE);

    // completion
    if (completion_count > 0)
    {
        // for each completion entry in our newest completion
        for (int i = 0; i < (int) completions[completion_count - 1].second.size() && i < height - 4; ++i)
        {
            std::string const & complete_entry =
                    matchmaker::at(completions[completion_count - 1].second.at(i));

            // draw complete_entry letter by letter
            for (int j = 0; j < (int) complete_entry.size(); ++j)
            {
                mvwaddch(
                    win,
                    i + 3,
                    j + 1,
                    complete_entry[j]
                );
            }
        }
    }

    wrefresh(win);
}
