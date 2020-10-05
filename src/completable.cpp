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



#include <memory>
#include <string>

#include <ncurses.h>

#include "CompletablePageAgent.h"
#include "IndicatorWindow.h"
#include "MatchmakerPage.h"
#include "PageDescriptionWindow.h"
#include "key_codes.h"
#include "matchmaker.h"
#include "completable_shell.h"



int main(int argc, char ** argv)
{
    if (argc == 2)
    {
        std::string const a1{argv[1]};
        if (a1 == "no_borders")
            AbstractWindow::global_borders_enabled() = false;
    }

#ifndef MM_DL
    matchmaker::set_library(nullptr); // disable dynamic loading, use linking instead
#endif

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

    auto page_desc_win = std::make_shared<PageDescriptionWindow>();
    auto indicator_win = std::make_shared<IndicatorWindow>();

    CompletablePageAgent cpa{page_desc_win, indicator_win};


//     SettingsPage page_s{{&page_desc_win, &indicator_win}};
//
//     page_c.set_left_neighbor(&page_s);
//     page_s.set_right_neighbor(&page_c);
//
// #ifdef MM_DL
//     MatchmakerPage page_m{{&page_desc_win, &indicator_win}};
//     page_s.set_left_neighbor(&page_m);
//     page_m.set_right_neighbor(&page_s);
//     AbstractPage::set_active_page(&page_m);
// #else
//     AbstractPage::set_active_page(&page_c);
// #endif
    AbstractPage::set_active_page((AbstractPage *) cpa().get());


    bool resized_draw{true};
    int ch{0};
    AbstractPage * active_page{nullptr};

    while (true)
    {
        active_page = AbstractPage::get_active_page();
        if (nullptr == active_page)
            break;

        // *** terminal resized? *************
        prev_root_y = root_y;
        prev_root_x = root_x;
        getmaxyx(stdscr, root_y, root_x);
        if (root_y != prev_root_y || root_x != prev_root_x)
        {
            active_page->resize();
            resized_draw = true;
        }
        // ***********************************

        active_page->draw(resized_draw);
        resized_draw = false;

        // a window is needed for keyboard input
        // page_desc_win is on all pages and is always enabled so it is the chosen one
        ch = wgetch(page_desc_win->get_WINDOW());

        if (ch == '$' || ch == '~' || ch == '`')
        {
            def_prog_mode();
            endwin();

            completable_shell();

            reset_prog_mode();
            resized_draw = true;
        }
        else if (ch == ESC)
        {
            nodelay(page_desc_win->get_WINDOW(), true);
            ch = wgetch(page_desc_win->get_WINDOW());
            nodelay(page_desc_win->get_WINDOW(), false);

            if (ch == ERR)
                break;
        }
        else
        {
            active_page->on_KEY(ch);
        }
    }

    endwin();


    matchmaker::unset_library();

    return 0;
}
