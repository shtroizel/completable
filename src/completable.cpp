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

#include "AbstractWindow.h"
#include "AntonymWindow.h"
#include "AttributeWindow.h"
#include "CompletionStack.h"
#include "CompletionWindow.h"
#include "FilterWindow.h"
#include "HelpWindow.h"
#include "InputWindow.h"
#include "LengthCompletionWindow.h"
#include "SynonymWindow.h"
#include "word_filter.h"



static int const ESC{27};
static int const TAB{9};

static int const RET(10);
static int const DEL{330};


void shell();


int main(int argc, char ** argv)
{
    if (argc == 2)
    {
        std::string const a1{argv[1]};
        if (a1 == "no_borders")
            AbstractWindow::borders() = false;
    }

    initscr();
    noecho();
    curs_set(FALSE);

    // window dimensions
    int root_y{0};
    int root_x{0};
    getmaxyx(stdscr, root_y, root_x);

    // differences to root_y ad root_x indicate window resize
    int prev_root_y{root_y};
    int prev_root_x{root_x};

    word_filter wf;
    CompletionStack cs{wf};
    std::stack<std::pair<std::string, AbstractWindow *>> ws; // for navigation with RET and DEL

    InputWindow input_win{cs, ws};
    input_win.enable();

    CompletionWindow completion_win{cs, ws, input_win};
    completion_win.enable();

    LengthCompletionWindow len_completion_win{cs, ws, input_win, completion_win};
    len_completion_win.enable();

    SynonymWindow syn_win{cs, ws, completion_win, wf};
    syn_win.enable();

    AntonymWindow ant_win{cs, ws, completion_win, len_completion_win, syn_win, wf};
    ant_win.enable();

    AttributeWindow pos_win{cs, ws, completion_win, len_completion_win, syn_win, ant_win};
    pos_win.enable();

    FilterWindow filter_win{cs, ws, wf};

    HelpWindow help_win{cs, ws};
    help_win.enable();
    AbstractWindow::set_active_window(&help_win);

    bool resized_draw{true};
    int ch{0};

    while (true)
    {
        // *** terminal resized? *************
        prev_root_y = root_y;
        prev_root_x = root_x;
        getmaxyx(stdscr, root_y, root_x);
        if (root_y != prev_root_y || root_x != prev_root_x)
        {
            input_win.resize();
            completion_win.resize();
            len_completion_win.resize();
            pos_win.resize();
            syn_win.resize();
            ant_win.resize();
            filter_win.resize();
            help_win.resize();

            resized_draw = true;
        }
        // ***********************************

        input_win.draw(resized_draw);
        completion_win.draw(resized_draw);
        len_completion_win.draw(resized_draw);
        pos_win.draw(resized_draw);
        syn_win.draw(resized_draw);
        ant_win.draw(resized_draw);
        filter_win.draw(resized_draw);
        help_win.draw(resized_draw);

        resized_draw = false;

        if (input_win.is_enabled())
            ch = wgetch(input_win.get_WINDOW());
        else if (filter_win.is_enabled())
            ch = wgetch(filter_win.get_WINDOW());

        if (ch == '$' || ch == '~' || ch == '`')
        {
            def_prog_mode();
            endwin();

            shell();

            reset_prog_mode();
            resized_draw = true;
        }
        else if (ch == RET)
        {
            if (help_win.is_enabled())
            {
                help_win.disable();
                AbstractWindow::set_active_window(&completion_win);
                input_win.mark_dirty();
                completion_win.mark_dirty();
                len_completion_win.mark_dirty();
                pos_win.mark_dirty();
                syn_win.mark_dirty();
                ant_win.mark_dirty();
            }
            else if (filter_win.is_enabled())
            {
                filter_win.toggle_hovered();
            }
            else
            {
                AbstractWindow * w = AbstractWindow::get_active_window();
                if (nullptr != w)
                {
                    w->on_RETURN();
                    input_win.mark_dirty();
                }
            }
        }
        else if (ch == DEL)
        {
            if (help_win.is_enabled())
            {
            }
            else if (filter_win.is_enabled())
            {
            }
            else
            {
                if (ws.size() == 0)
                    continue;

                auto & [s, w] = ws.top();

                while (cs.count() > 1)
                    cs.pop();

                for (auto ch : s)
                    cs.push(ch);

                ws.pop();

                AbstractWindow::set_active_window(w);
                input_win.mark_dirty();
            }
        }
        else if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b')
        {
            if (help_win.is_enabled())
            {
            }
            else if (filter_win.is_enabled())
            {
            }
            else
            {
                int old_count = cs.count();
                cs.pop();
                int new_count = cs.count();
                if (new_count != old_count)
                    input_win.mark_dirty();
            }
        }
        else if (ch == TAB)
        {
            if (help_win.is_enabled())
            {
            }
            else if (filter_win.is_enabled())
            {
            }
            else
            {
                decltype(cs) const & ccs = cs;

                auto const & c = ccs.top();
                std::string const & prefix = c.prefix;

                if (c.standard_completion.size() > 0)
                {
                    std::string const & first_entry = matchmaker::at(c.standard_completion[0]);

                    // find out the "target_completion_count" or the completion count after skipping
                    // by common characters
                    int target_completion_count = ccs.count() - 1;
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
        }
        else if (ch == KEY_LEFT)
        {
            if (help_win.is_enabled())
            {
            }
            else if (filter_win.is_enabled())
            {
            }
            else
            {
                if (ant_win.is_active())
                    AbstractWindow::set_active_window(&len_completion_win);
                else
                    if (len_completion_win.is_active())
                        AbstractWindow::set_active_window(&syn_win);
                    else
                        if (syn_win.is_active())
                            AbstractWindow::set_active_window(&completion_win);
                        else
                            if (completion_win.is_active())
                                AbstractWindow::set_active_window(&ant_win);
                            else
                                AbstractWindow::set_active_window(&completion_win);
            }
        }
        else if (ch == KEY_RIGHT)
        {
            if (help_win.is_enabled())
            {
            }
            else if (filter_win.is_enabled())
            {
            }
            else
            {
                if (completion_win.is_active())
                    AbstractWindow::set_active_window(&syn_win);
                else
                    if (syn_win.is_active())
                        AbstractWindow::set_active_window(&len_completion_win);
                    else
                        if (len_completion_win.is_active())
                            AbstractWindow::set_active_window(&ant_win);
                        else
                            if (ant_win.is_active())
                                AbstractWindow::set_active_window(&completion_win);
                            else
                                AbstractWindow::set_active_window(&ant_win);
            }
        }
        else if (ch == KEY_F(1)  ||
                 ch == KEY_F(2)  ||
                 ch == KEY_F(3)  ||
                 ch == KEY_F(4)  ||
                 ch == KEY_F(5)  ||
                 ch == KEY_F(6)  ||
                 ch == KEY_F(7)  ||
                 ch == KEY_F(8)  ||
                 ch == KEY_F(9)  ||
                 ch == KEY_F(10) ||
                 ch == KEY_F(11) ||
                 ch == KEY_F(12))
        {
            if (help_win.is_enabled())
                continue;

            filter_win.set_enabled(!filter_win.is_enabled());

            if (filter_win.is_enabled())
            {
                AbstractWindow::set_active_window(&filter_win);
            }
            else
            {
                AbstractWindow::set_active_window(&completion_win);
                input_win.mark_dirty();
                completion_win.mark_dirty();
                len_completion_win.mark_dirty();
                pos_win.mark_dirty();
                syn_win.mark_dirty();
                ant_win.mark_dirty();
            }
        }
        else if (ch == ESC)
        {
            if (filter_win.is_enabled())
            {
                filter_win.disable();
                AbstractWindow::set_active_window(&completion_win);
                input_win.mark_dirty();
                completion_win.mark_dirty();
                len_completion_win.mark_dirty();
                pos_win.mark_dirty();
                syn_win.mark_dirty();
                ant_win.mark_dirty();
            }
            else
            {
                nodelay(input_win.get_WINDOW(), true);
                ch = wgetch(input_win.get_WINDOW());
                nodelay(input_win.get_WINDOW(), false);

                if (ch == ERR)
                    break;
            }
        }
        else if (ch > 31 && ch < 127) // printable ascii
        {
            if (help_win.is_enabled())
            {
            }
            if (filter_win.is_enabled())
            {
            }
            else
            {
                bool old_count = cs.count();
                cs.push(ch);
                if (cs.count() != old_count)
                    input_win.mark_dirty();
            }
        }
        else
        {
            AbstractWindow * w = AbstractWindow::get_active_window();
            if (nullptr != w)
                w->on_KEY(ch);
        }
    }

    endwin();

    return 0;
}


void shell()
{
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
                      << "{ prefix the word with '!' for completion                                     }\n"
                      << "{ use  :it <index> <count>        to iterate <count> words from <index>       }\n"
                      << "{ use  :pos <word>                for parts of speech of <word>               }\n"
                      << "{ use  :s <word>                  for synonyms of <word>                      }\n"
                      << "{ use  :a <word>                  for antonyms of <word>                      }\n"
                      << "{ use  :itl <index> <count>       like ':it' but uses length indexes          }\n"
                      << "{ use  :len                       to list length index offsets                }\n"
                      << "{ use  :help                      to toggle help                              }\n"
                      << "matchmaker (" << matchmaker::size() << ") $  ";
        else
            std::cout << "matchmaker (" << matchmaker::size() << ") { enter :help for help } $  ";

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
            if (words[0] == ":q")
            {
                exit(EXIT_SUCCESS);
            }
            else if (words[0] == ":len")
            {
                std::cout << "The following length indexes can be used with ':itl'" << std::endl;

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
            else if (words[0][0] == '!')
            {
                int completion_start{0};
                int completion_length{0};
                auto start = std::chrono::high_resolution_clock::now();
                matchmaker::complete(words[0].substr(1), completion_start, completion_length);
                auto stop = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                std::cout << "completion (" << completion_length << ") : ";
                for (int i = completion_start; i < completion_start + completion_length; ++i)
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
                if (index < matchmaker::size())
                {
                    std::cout << index << "], length[" << matchmaker::as_longest(index) << "] :  '"
                              << matchmaker::at(index) << "' ";
                    if (!found)
                        std::cout << "(index if existed) ";
                }
                else
                {
                    std::cout << matchmaker::size() << "], (would be new last word)";
                }
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
                if (index == matchmaker::size())
                {
                    std::cout << matchmaker::size() << "], (would be new last word)" << std::endl;
                    continue;
                }
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
                if (index == matchmaker::size())
                {
                    std::cout << matchmaker::size() << "], (would be new last word)" << std::endl;
                    continue;
                }
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
                    if (index == matchmaker::size())
                    {
                        std::cout << matchmaker::size() << "], (would be new last word)" << std::endl;
                        continue;
                    }
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

            if (words[0] == ":itl")
            {
                for (int i = start; i < (int) matchmaker::by_longest().size() && i < start + count; ++i)
                    std::cout << "       [" << std::setw(MAX_INDEX_DIGITS) << matchmaker::by_longest()[i]
                              << "], length[" << std::setw(MAX_INDEX_DIGITS) << i << "]  "
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
                    std::cout << "       [" << std::setw(MAX_INDEX_DIGITS) << i << "], length["
                              << std::setw(MAX_INDEX_DIGITS) << matchmaker::as_longest(i) << "] :  '"
                              << str << "' accessed in " << duration.count() << " microseconds\n";
                }
                std::cout << std::flush;
            }
        }
    }
}
