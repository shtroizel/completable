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



#include "SettingsWindow.h"

#include <iostream>
#include <string>
#include <vector>

#include <ncurses.h>

#include "InputWindow.h"
#include "Layer.h"
#include "EnablednessSetting.h"
#include "VisibilityAspect.h"



std::string SettingsWindow::title()
{
    static std::string const t{"Enabledness"};
    return t;
}


void SettingsWindow::resize_hook()
{
    height = 5;
    width = 53;

    // center window
    y = (root_y - height) / 2;
    x = (root_x - width) / 2;
}


void SettingsWindow::draw_hook()
{
    static int const right_align =
        [&]()
        {
            int max_len{0};
            for (auto s : EnablednessSetting::variants())
                if (max_len < (int) s.as_string().length())
                    max_len = s.as_string().length();
            return max_len + 7;
        }();


    for (auto setting : EnablednessSetting::variants())
    {
        // print setting name
        mvwprintw(
            w,
            setting.as_index() + 2,
            right_align - setting.as_string().length(),
            setting.as_string().c_str()
        );

        mvwprintw(w, setting.as_index() + 2, right_align, ":");

        // print enabledness
        if (setting.as_index() == selection)
            wattron(w, A_REVERSE);
        mvwprintw(w, setting.as_index() + 2, right_align + 2, setting.as_enabledness().as_string().c_str());
        wattroff(w, A_REVERSE);

        // clear space after "Enabled" since "Disabled" is a char longer
        mvwaddch(
            w,
            setting.as_index() + 2,
            right_align + 2 + setting.as_enabledness().as_string().size(),
            ' '
        );
    }
}


Layer::Type SettingsWindow::layer() const
{
    return Layer::Bottom::grab();
}


void SettingsWindow::on_RETURN()
{
    auto setting = EnablednessSetting::from_index(selection);

    int index = setting.as_enabledness().as_index() + 1;
    if (index >= (int) Enabledness::variants().size())
        index = 0;

    setting.set_enabledness(Enabledness::from_index(index));
    mark_dirty();
}


void SettingsWindow::on_KEY_UP()
{
    if (selection > 0)
    {
        --selection;
        mark_dirty();
    }
}


void SettingsWindow::on_KEY_DOWN()
{
    if (selection < (int) EnablednessSetting::variants().size() - 1)
    {
        ++selection;
        mark_dirty();
    }
}
