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
    int complete_width{0};
    for (int i = 0; i < matchmaker::size(); ++i)
        if ((int) matchmaker::at(i).size() > complete_width)
            complete_width = matchmaker::at(i).size();
    complete_width += 2;

    int complete_height{root_y};

    WINDOW * complete_win = newwin(complete_height, complete_width, 0, 0);
    keypad(complete_win, true);

    completion completions[MAX_COMPLETIONS];
    completions[0].start = 0;
    completions[0].display_start = 0;
    completions[0].length = matchmaker::size();
    int completion_count{1};

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
        }

        draw_complete_win(complete_height, complete_width, completion_count, completions, complete_win);

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
        else if (ch == 9) // TAB
        {
            if (completion_count > 0)
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
            if (completion_count > 0)
            {
                completion & c = completions[completion_count - 1];
                if (c.display_start > c.start)
                    --c.display_start;
            }
        }
        else if (ch == KEY_DOWN)
        {
            if (completion_count > 0)
            {
                completion & c = completions[completion_count - 1];
                if (c.display_start < c.start + c.length)
                    ++c.display_start;
            }
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
    if (completion_count > 0)
        completions[completion_count].prefix = completions[completion_count - 1].prefix;
    else
        completions[completion_count].prefix.clear();

    // add new character to prefix
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
    if (completion_count > 0)
    {
        completion & cur_completion = completions[completion_count - 1];

        int length = cur_completion.length - (cur_completion.display_start - cur_completion.start);

        // for each completion entry in our newest completion
        for (int i = 0; i < length && i < height - 4; ++i)
        {
            std::string const & complete_entry = matchmaker::at(cur_completion.display_start + i);

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
