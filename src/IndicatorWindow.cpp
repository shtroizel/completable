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



#include "IndicatorWindow.h"

#include <ncurses.h>

#include "AbstractTab.h"
#include "EnablednessSetting.h"
#include "Layer.h"



std::string IndicatorWindow::title()
{
    static std::string const t;
    return t;
}


void IndicatorWindow::resize_hook()
{
    height = 3;
    width = 17;
    y = 0;
    x = root_x - width;
}


void IndicatorWindow::draw_hook()
{
    auto active_tab = AbstractTab::get_active_tab();
    if (active_tab.is_nil())
        return;

    int abbreviation_x = 0;
    for (auto tab : Tab::variants())
    {
        AbstractTab * abstract_tab = tab.as_AbstractTab();
        if (nullptr == abstract_tab)
            continue;

        abbreviation_x = width - 3 - (5 * abstract_tab->get_indicator_position());

        if (tab == active_tab)
        {
            if (EnablednessSetting::Borders::grab().as_enabledness() == Enabledness::Enabled::grab())
            {
                mvwaddch(w, 0, abbreviation_x - 2, ACS_ULCORNER);
                mvwaddch(w, 0, abbreviation_x - 1, ACS_HLINE);
                mvwaddch(w, 0, abbreviation_x, ACS_HLINE);
                mvwaddch(w, 0, abbreviation_x + 1, ACS_HLINE);
                mvwaddch(w, 0, abbreviation_x + 2, ACS_URCORNER);
                mvwaddch(w, 1, abbreviation_x - 2, ACS_VLINE);
                mvwaddch(w, 1, abbreviation_x + 2, ACS_VLINE);
                mvwaddch(w, 2, abbreviation_x - 2, ACS_LLCORNER);
                mvwaddch(w, 2, abbreviation_x - 1, ACS_HLINE);
                mvwaddch(w, 2, abbreviation_x, ACS_HLINE);
                mvwaddch(w, 2, abbreviation_x + 1, ACS_HLINE);
                mvwaddch(w, 2, abbreviation_x + 2, ACS_LRCORNER);
            }
            else
            {
                wattron(w, A_REVERSE);
            }
        }
        else
        {
            mvwaddch(w, 0, abbreviation_x - 2, ' ');
            mvwaddch(w, 0, abbreviation_x - 1, ' ');
            mvwaddch(w, 0, abbreviation_x, ' ');
            mvwaddch(w, 0, abbreviation_x + 1, ' ');
            mvwaddch(w, 0, abbreviation_x + 2, ' ');
            mvwaddch(w, 1, abbreviation_x - 2, ' ');
            mvwaddch(w, 1, abbreviation_x + 2, ' ');
            mvwaddch(w, 2, abbreviation_x - 2, ' ');
            mvwaddch(w, 2, abbreviation_x - 1, ' ');
            mvwaddch(w, 2, abbreviation_x, ' ');
            mvwaddch(w, 2, abbreviation_x + 1, ' ');
            mvwaddch(w, 2, abbreviation_x + 2, ' ');
        }

        char abbreviation = 'X';
        if (tab.as_string().length() > 0)
            abbreviation = tab.as_string()[0];

        if (abbreviation >= 'a' && abbreviation <= 'z')
            abbreviation -= 'a' - 'A';

        mvwaddch(w, 1, abbreviation_x, abbreviation);
        wattroff(w, A_REVERSE);
    }
}


Layer::Type IndicatorWindow::layer() const
{
    return Layer::Bottom::grab();
}
