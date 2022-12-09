#include <memory>
#include <string>
#include <iostream>

#include <ncurses.h>

#include "CompletableTabAgent.h"
#include "Settings.h"
#include "IndicatorWindow.h"
#include "MatchmakerTab.h"
#include "MatchmakerTabAgent.h"
#include "TabDescriptionWindow.h"
#include "SettingsTabAgent.h"
#include "key_codes.h"
#include "matchmaker.h"
#include "completable_shell.h"



int main(int argc, char ** argv)
{
    if (argc == 2)
    {
        std::string const a1{argv[1]};
        if (a1 == "borders_disabled")
            EnablednessSetting::Borders::grab().set_enabledness(Enabledness::Disabled::grab());
    }

#ifndef MM_DYNAMIC_LOADING
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

    auto tab_desc_win = std::make_shared<TabDescriptionWindow>();
    auto indicator_win = std::make_shared<IndicatorWindow>();

    CompletableTabAgent cta{tab_desc_win, indicator_win};

    SettingsTabAgent sta{tab_desc_win, indicator_win};

    cta()->set_left_neighbor(sta()->as_handle());
    sta()->set_right_neighbor(cta()->as_handle());

#ifdef MM_DYNAMIC_LOADING
    MatchmakerTabAgent mta{tab_desc_win, indicator_win};
    mta()->set_right_neighbor(sta()->as_handle());
    sta()->set_left_neighbor(mta()->as_handle());
    AbstractTab::set_active_tab(mta()->as_handle());
#else
    AbstractTab::set_active_tab(cta()->as_handle());
#endif


    bool resized_draw{true};
    int ch{0};
    AbstractTab * active_tab{nullptr};

    while (true)
    {
        if (AbstractTab::get_active_tab().is_nil()) // should be impossible
            break;

        active_tab = AbstractTab::get_active_tab().as_AbstractTab();
        if (nullptr == active_tab) // should be impossible
            break;

        // *** terminal resized? *************
        prev_root_y = root_y;
        prev_root_x = root_x;
        getmaxyx(stdscr, root_y, root_x);
        if (root_y != prev_root_y || root_x != prev_root_x)
        {
            active_tab->resize();
            resized_draw = true;
        }
        // ***********************************

        active_tab->draw(resized_draw);
        resized_draw = false;

        // a window is needed for keyboard input
        // tab_desc_win is on all tabs and is always enabled so it is the chosen one
        ch = wgetch(tab_desc_win->get_WINDOW());

        // enter shell mode?
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
            nodelay(tab_desc_win->get_WINDOW(), true);
            ch = wgetch(tab_desc_win->get_WINDOW());
            nodelay(tab_desc_win->get_WINDOW(), false);

            if (ch == ERR)
                break;
        }
        else
        {
            active_tab->on_KEY(ch);
        }
    }

    endwin();


    matchmaker::unset_library();

    return 0;
}
