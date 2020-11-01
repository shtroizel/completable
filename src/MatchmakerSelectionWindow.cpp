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



#include "MatchmakerSelectionWindow.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include <ncurses.h>

#include "EnablednessSetting.h"
#include "InputWindow.h"
#include "Layer.h"
#include "VisibilityAspect.h"
#include "matchmaker.h"
#include "exec_long_task_with_busy_animation.h"



void MatchmakerSelectionWindow::set_content(std::vector<std::string> new_content)
{
    content = new_content;
    mark_dirty();
}


std::string MatchmakerSelectionWindow::title()
{
    static std::string const t{""};
    return t;
}


void MatchmakerSelectionWindow::resize_hook()
{
    y = 5; // overlap with last line of MatchmakerLocationWindow
    x = 0;

    height = root_y - y;
    width = root_x;
}


void MatchmakerSelectionWindow::draw_hook()
{
    if (EnablednessSetting::Borders::grab().as_enabledness() == Enabledness::Enabled::grab())
    {
        box(w, 0, 0);
        mvwaddch(w, 0, 0, ACS_LTEE);
        mvwaddch(w, 0, width - 1, ACS_RTEE);
    }

    int display_count = (int) content.size() - selected;

    int i = 0;
    for (; i < display_count && i < height - 2; ++i)
    {
        std::string dictionary = content[selected + i];

        if (i == 0)
            wattron(w, A_REVERSE);

        int j = 0;
        for (; j < (int) dictionary.length() && j < width - 2; ++j)
            mvwaddch(w, i + 1, j + 1, dictionary[j]);

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


Layer::Type MatchmakerSelectionWindow::layer() const
{
    return Layer::Bottom::grab();
}


void MatchmakerSelectionWindow::load_currently_selected()
{
    if (content.size() > 0)
    {
        exec_long_task_with_busy_animation(
            [&]() { matchmaker::set_library(content.at(selected).c_str()); },
            *this
        );
        mark_dirty();
    }
}


void MatchmakerSelectionWindow::select_previous()
{
    if (selected > 0)
    {
        --selected;
        mark_dirty();
    }
}


void MatchmakerSelectionWindow::select_next()
{
    if (selected < (int) content.size() - 1)
    {
        ++selected;
        mark_dirty();
    }
}
