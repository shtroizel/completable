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

#include "CompletionStack.h"



static int const PAGE_UP{339};
static int const PAGE_DOWN{338};
static int const TAB{9};
static int const HOME{262};
static int const END{360};

// minimum required terminal height
static int const MIN_ROOT_Y{30};

// minimum required terminal width
static int const MIN_ROOT_X{80};


// TODO recode activation concept
enum class Win{ Completion, LengthCompletion };
Win active_win = Win::Completion;


class AbstractWindow
{
public:
    // one is enough
    AbstractWindow(AbstractWindow const &) = delete;
    AbstractWindow & operator=(AbstractWindow const &) = delete;

    AbstractWindow() {}
    virtual ~AbstractWindow() { delwin(w); }

    void clear() { wclear(w); wrefresh(w); }

    void resize()
    {
        if (nullptr != w)
        {
            clear();
            delwin(w);
        }

        getmaxyx(stdscr, root_y, root_x);

        resize_hook();

        w = newwin(height, width, y, x);
        keypad(w, true);
    }

    void draw(CompletionStack const & cs)
    {
        wclear(w);

        // check terminal for minimum size requirement
        if (root_y < MIN_ROOT_Y || root_x < MIN_ROOT_X)
        {
            wrefresh(w);
            return;
        }

        // border
        box(w, 0, 0);

        // title
        int const indent{width / 3 - (int) title().size() / 2};
        mvwaddch(w, 0, indent - 1, ' ');
        for (int i = 0; i < (int) title().size(); ++i)
            mvwaddch(w, 0, i + indent, title()[i]);
        mvwaddch(w, 0, title().size() + indent, ' ');

        // window specific drawing
        draw_hook(cs);

        wrefresh(w);
    }

    int get_height() const { return height; }
    int get_width() const { return width; }
    int get_y() const { return y; }
    int get_x() const { return x; }
    WINDOW * get_WINDOW() const { return w; }

private:
    virtual std::string const & title() const = 0;
    virtual void resize_hook() = 0;
    virtual void draw_hook(CompletionStack const & cs) = 0;

protected:
    WINDOW * w{nullptr};
    int root_y{0};
    int root_x{0};
    int height{0};
    int width{0};
    int y{0};
    int x{0};
};


class PropertyWindow : public AbstractWindow
{
    void draw_hook(CompletionStack const & cs) override
    {
        int selected{0};
        if (active_win == Win::Completion)
        {
            selected = cs.top().display_start;
        }
        else if (active_win == Win::LengthCompletion)
        {
            int length_completion_index = cs.top().len_display_start;
            if (length_completion_index >= (int) cs.top().length_completion.size())
                return;
            if (length_completion_index < 0)
                return;
            int long_index = cs.top().length_completion[length_completion_index];
            selected = matchmaker::from_longest(long_index);
        }
        else
        {
            return;
        }

        draw_hook(selected);
    }

    virtual void draw_hook(int selected) = 0;
};


class InputWindow : public AbstractWindow
{
public:
    InputWindow() : AbstractWindow() { keypad(w, true); }

private:
    std::string const & title() const override { static std::string const t{"Input"}; return t; }

    void resize_hook() override
    {
        height = 3;
        width = root_x / 2;
        y = 0;
        x = root_x / 2 - width / 2;
    }

    void draw_hook(CompletionStack const & cs) override
    {
        std::string const & prefix = cs.top().prefix;
        for (int x = 0; x < width - 2 && x < (int) prefix.size(); ++x)
            mvwaddch(w, 1, x + 1, prefix[x]);
    }
};


class CompletionWindow : public AbstractWindow
{
    std::string const & title() const override { static std::string const t{"Completion"}; return t; }

    void resize_hook() override
    {
        int combined_height = root_y - 5 - y;
        height = y + ((combined_height * 9) / 17);
        width = root_x / 2;
        y = 3;
        x = 0;
    }

    void draw_hook(CompletionStack const & cs) override
    {
        auto const & cur_completion = cs.top();
        int length = cur_completion.length - (cur_completion.display_start - cur_completion.start);
        for (int i = 0; i < length && i < height - 2; ++i)
        {
            std::string const & complete_entry = matchmaker::at(cur_completion.display_start + i);

            if (active_win == Win::Completion && i == 0)
                wattron(w, A_REVERSE);

            // draw complete_entry letter by letter
            for (int j = 0; j < (int) complete_entry.size() && j < width - 2; ++j)
            {
                mvwaddch(
                    w,
                    i + 1,
                    j + 1,
                    complete_entry[j]
                );
            }

            if (active_win == Win::Completion && i == 0)
                wattroff(w, A_REVERSE);
        }
    }
};


class LengthCompletionWindow : public AbstractWindow
{
public:
    LengthCompletionWindow(CompletionWindow const & win) : AbstractWindow(), completion_win{win} {}

private:
    std::string const & title() const override
    {
        static std::string const t{"Length Completion"};
        return t;
    }

    void resize_hook() override
    {
        y = completion_win.get_y() + completion_win.get_height();
        x = 0;
        height = root_y - y - 5;
        width = completion_win.get_width();
    }

    void draw_hook(CompletionStack const & cs) override
    {
        auto const & cur_completion = cs.top();
        int length = cur_completion.length_completion.size() - cur_completion.len_display_start;
        if (length < 0)
            return;

        int long_index{0};

        for (int i = 0; i < length && i < height - 2; ++i)
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
                wattron(w, A_REVERSE);

            // draw entry
            for (int j = 0; j < (int) complete_entry.size() && j < width - 2; ++j)
            {
                mvwaddch(
                    w,
                    i + 1,
                    j + 1,
                    complete_entry[j]
                );
            }

            if (active_win == Win::LengthCompletion && i == 0)
                wattroff(w, A_REVERSE);
        }
    }

private:
    CompletionWindow const & completion_win;
};


class PartsOfSpeechWindow : public PropertyWindow
{
    std::string const & title() const override
    {
        static std::string const t{"Parts of Speech"};
        return t;
    }

    void resize_hook() override
    {
        height = 5;
        width = root_x;
        y = root_y - height;
        x = 0;
    }

    void draw_hook(int selected) override
    {
        int const cell_width{16};

        auto const & flagged_pos = matchmaker::flagged_parts_of_speech(selected);

        int x = 1;
        int i = 0;
        int y = 1;
        while (i < (int) matchmaker::all_parts_of_speech().size())
        {
            if (flagged_pos[i] != 0)
                wattron(w, A_REVERSE);

            for(
                int j = 0;
                j < (int) matchmaker::all_parts_of_speech()[i].size() && j < x + cell_width - 2;
                ++j
            )
                mvwaddch(w, y, j + x, matchmaker::all_parts_of_speech()[i][j]);

            if (flagged_pos[i] != 0)
                wattroff(w, A_REVERSE);

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
    }
};


class SynonymWindow : public PropertyWindow
{
public:
    SynonymWindow(CompletionWindow const & win) : PropertyWindow(), completion_win{win} {}

private:
    std::string const & title() const override
    {
        static std::string const t{"Synonyms"};
        return t;
    }

    void resize_hook() override
    {
        height = completion_win.get_height();
        width = completion_win.get_width() + root_x % 2;
        y = completion_win.get_y();
        x = completion_win.get_width();
    }

    void draw_hook(int selected) override
    {
        auto const & synonyms = matchmaker::synonyms(selected);
        for (int i = 0; i < (int) synonyms.size() && i < height - 2; ++i)
        {
            std::string const & syn = matchmaker::at(synonyms[i]);
            for (int j = 0; j < (int) syn.length() && j < width - 2; ++j)
            {
                mvwaddch(
                    w,
                    i + 1,
                    j + 1,
                    syn[j]
                );
            }
        }
    }

private:
    CompletionWindow const & completion_win;
};


class AntonymWindow : public PropertyWindow
{
public:
    AntonymWindow(LengthCompletionWindow const & win) : PropertyWindow(), len_completion_win{win} {}

private:
    std::string const & title() const override
    {
        static std::string const t{"Antonyms"};
        return t;
    }

    void resize_hook() override
    {
        height = len_completion_win.get_height();
        width = len_completion_win.get_width() + root_x % 2;
        y = len_completion_win.get_y();
        x = len_completion_win.get_width();
    }

    void draw_hook(int selected) override
    {
        auto const & antonyms = matchmaker::antonyms(selected);
        for (int i = 0; i < (int) antonyms.size() && i < height - 2; ++i)
        {
            std::string const & ant = matchmaker::at(antonyms[i]);
            for (int j = 0; j < (int) ant.length() && j < width - 2; ++j)
            {
                mvwaddch(
                    w,
                    i + 1,
                    j + 1,
                    ant[j]
                );
            }
        }
    }

private:
    LengthCompletionWindow const & len_completion_win;
};



void shell();



int main()
{
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

    InputWindow input_win;
    input_win.resize();

    CompletionWindow completion_win;
    completion_win.resize();

    LengthCompletionWindow len_completion_win{completion_win};
    len_completion_win.resize();

    PartsOfSpeechWindow pos_win;
    pos_win.resize();

    SynonymWindow syn_win{completion_win};
    syn_win.resize();

    AntonymWindow ant_win{len_completion_win};
    ant_win.resize();

    CompletionStack cs;

    int ch{0};

    while (1)
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
        }
        // ***********************************

        input_win.draw(cs);
        completion_win.draw(cs);
        len_completion_win.draw(cs);
        pos_win.draw(cs);
        syn_win.draw(cs);
        ant_win.draw(cs);

        ch = wgetch(input_win.get_WINDOW());

        if (ch == '$' || ch == '~' || ch == '`')
        {
            def_prog_mode();
            endwin();

            shell();

            reset_prog_mode();
            refresh();
        }
        else if (ch > 31 && ch < 127) // printable ascii
        {
            cs.push(ch);
        }
        else if (ch == KEY_BACKSPACE)
        {
            cs.pop();
        }
        else if (ch == TAB)
        {
            // TODO FIXME BROKEN
//             decltype(cs) const & ccs = cs;
//
//             auto const & cur_completion = ccs.top();
//             std::string const & prefix = cur_completion.prefix;
//             if (cur_completion.length > 0)
//             {
//                 std::string const & first_entry =
//                         matchmaker::at(cur_completion.start);
//
//
//                 // find out the "target_completion_count" or the completion count after skipping
//                 // by common characters
//                 int target_completion_count = ccs.count();
//                 bool ok = first_entry.size() > prefix.size();
//                 while (ok)
//                 {
//                     for (
//                         int i = cur_completion.start;
//                         ok && i < cur_completion.start + cur_completion.length;
//                         ++i
//                     )
//                     {
//                         std::string const & entry = matchmaker::at(i);
//
//                         if ((int) entry.size() < target_completion_count)
//                             ok = false;
//                         else if ((int) first_entry.size() < target_completion_count)
//                             ok = false;
//                         else if (entry[target_completion_count] != first_entry[target_completion_count])
//                             ok = false;
//                     }
//
//                     if (ok)
//                         ++target_completion_count;
//                 }
//
//                 // grow up to the target completion count
//                 for (int i = (int) prefix.size(); i < target_completion_count; ++i)
//                     cs.push(first_entry[i]);
//             }
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
            auto & c = cs.top();

            if (active_win == Win::Completion && c.display_start > c.start)
                --c.display_start;
            else if (active_win == Win::LengthCompletion && c.len_display_start > 0)
                --c.len_display_start;
        }
        else if (ch == KEY_DOWN)
        {
            auto & c = cs.top();

            if (active_win == Win::Completion && c.display_start < c.start + c.length - 1)
                ++c.display_start;
            else if (active_win == Win::LengthCompletion &&
                    c.len_display_start < (int) c.length_completion.size() - 1)
                ++c.len_display_start;
        }
        else if (ch == PAGE_UP)
        {
            auto & c = cs.top();

            if (active_win == Win::Completion)
            {
                c.display_start -= completion_win.get_height() - 2;
                if (c.display_start < c.start)
                    c.display_start = c.start;
            }
            else if (active_win == Win::LengthCompletion)
            {
                c.len_display_start -= len_completion_win.get_height() - 2;
                if (c.len_display_start < 0)
                    c.len_display_start = 0;
            }
        }
        else if (ch == PAGE_DOWN)
        {
            auto & c = cs.top();

            if (active_win == Win::Completion)
            {
                c.display_start += completion_win.get_height() - 2;
                if (c.display_start >= c.start + c.length)
                    c.display_start = c.start + c.length - 1;
            }
            else if (active_win == Win::LengthCompletion)
            {
                c.len_display_start += len_completion_win.get_height() - 2;
                if (c.len_display_start >= (int) c.length_completion.size())
                    c.len_display_start = (int) c.length_completion.size() - 1;
            }
        }
        else if (ch == HOME)
        {
            auto & c = cs.top();
            if (active_win == Win::Completion)
                c.display_start = c.start;
            else if (active_win == Win::LengthCompletion)
                c.len_display_start = 0;
        }
        else if (ch == END)
        {
            auto & c = cs.top();
            if (active_win == Win::Completion)
                c.display_start = c.start + c.length - 1;
            else if (active_win == Win::LengthCompletion)
                c.len_display_start = c.length_completion.size() - 1;
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
                      << "{ prefix the word with ':' for completion                                     }\n"
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
                CompletionStack::completion c;
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
                std::cout << index << "], length[" << matchmaker::as_longest(index) << "] :  '"
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
