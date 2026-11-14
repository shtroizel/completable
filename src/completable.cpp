#include <memory>
#include <string>
#include <iostream>
#include <vector>

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



static void print_usage()
{
    std::cout << "usage: completable [options] [borders_disabled]\n"
              << "    -c <command>    run a shell command and exit without curses (repeat to run several)\n"
              << "    -s              start in shell mode, reading commands from stdin\n"
              << "    -n              leave timing out of shell output\n"
#ifdef MM_DYNAMIC_LOADING
              << "    -l <library>    load a matchmaker library at startup\n"
#endif
              << "    -h              print this message" << std::endl;
}



int main(int argc, char ** argv)
{
    std::vector<std::string> commands;
    bool shell{false};
    bool show_timing{true};
    std::string library;

    for (int i = 1; i < argc; ++i)
    {
        std::string const arg{argv[i]};
        if (arg == "borders_disabled")
        {
            EnablednessSetting::Borders::grab().set_enabledness(Enabledness::Disabled::grab());
        }
        else if (arg == "-c" && i + 1 < argc)
        {
            commands.push_back(argv[++i]);
        }
        else if (arg == "-s")
        {
            shell = true;
        }
        else if (arg == "-n")
        {
            show_timing = false;
        }
#ifdef MM_DYNAMIC_LOADING
        else if (arg == "-l" && i + 1 < argc)
        {
            library = argv[++i];
        }
#endif
        else if (arg == "-h")
        {
            print_usage();
            return 0;
        }
        else
        {
            std::cerr << "invalid argument: " << arg << std::endl;
            print_usage();
            return 1;
        }
    }

#ifdef MM_DYNAMIC_LOADING
    if (!library.empty())
    {
        char const * error = matchmaker::set_library(library.c_str());
        if (nullptr != error)
        {
            std::cerr << "failed to load " << library << ": " << error << std::endl;
            return 1;
        }
    }
#else
    matchmaker::set_library(nullptr); // disable dynamic loading, use linking instead
#endif

    if (shell || !commands.empty())
    {
        completable_shell(commands, show_timing);
        matchmaker::unset_library();
        return 0;
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

            if (completable_shell({}, show_timing))
                break;

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
