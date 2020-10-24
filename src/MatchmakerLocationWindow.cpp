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



#include "MatchmakerLocationWindow.h"

#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include <ncurses.h>

#include "EnablednessSetting.h"
#include "InputWindow.h"
#include "Layer.h"
#include "MatchmakerSelectionWindow.h"
#include "VisibilityAspect.h"



MatchmakerLocationWindow::MatchmakerLocationWindow(MatchmakerSelectionWindow & sel_win)
    : mm_sel_win{sel_win}
{
}


std::string MatchmakerLocationWindow::title()
{
    static std::string const t{"search location"};
    return t;
}


void MatchmakerLocationWindow::resize_hook()
{
    height = 3;
    width = root_x;

    y = 3;
    x = 0;
}


void MatchmakerLocationWindow::draw_hook()
{
    int i = 0;
    for (; i < (int) search_prefix.size() && i < width - 2; ++i)
        mvwaddch(w, 1, i + 1, search_prefix[i]);

    for (; i < width - 2; ++i)
        mvwaddch(w, 1, i + 1, ' ');
}


Layer::Type MatchmakerLocationWindow::layer() const
{
    return Layer::Bottom::grab();
}


void MatchmakerLocationWindow::on_TAB()
{
    std::vector<std::string> dictionaries;
    try
    {
        for (auto const & entry : std::filesystem::recursive_directory_iterator(search_prefix))
        {
            if (entry.is_regular_file())
            {
                if (strcmp(entry.path().filename().c_str(), "libmatchmaker.so") == 0)
                {
                    std::string dict = entry.path();
                    dictionaries.push_back(dict);
                }
            }
        }
    }
    catch (std::filesystem::__cxx11::filesystem_error const &) {}

    mm_sel_win.set_content(dictionaries);
}


void MatchmakerLocationWindow::on_printable_ascii(int key)
{
    search_prefix += (char) key;
    mark_dirty();
}


void MatchmakerLocationWindow::on_BACKSPACE()
{
    if (search_prefix.size() > 0)
    {
        search_prefix.pop_back();
        mark_dirty();
    }
}


void MatchmakerLocationWindow::on_RETURN()
{
    mm_sel_win.on_RETURN();
}


void MatchmakerLocationWindow::on_KEY_UP()
{
    mm_sel_win.on_KEY_UP();
}


void MatchmakerLocationWindow::on_KEY_DOWN()
{
    mm_sel_win.on_KEY_DOWN();
}
