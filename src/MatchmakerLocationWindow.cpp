#include "MatchmakerLocationWindow.h"

#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include <ncurses.h>

#include "Settings.h"
#include "InputWindow.h"
#include "Layer.h"
#include "MatchmakerSelectionWindow.h"
#include "VisibilityAspect.h"
#include "exec_long_task_with_busy_animation.h"



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
    if (EnablednessSetting::Borders::grab().as_enabledness() == Enabledness::Enabled::grab())
    {
        mvwaddch(w, height - 1, 0, ACS_LTEE);
        mvwaddch(w, height - 1, width - 1, ACS_RTEE);
    }

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
    exec_long_task_with_busy_animation(
        [&]()
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
        },
        mm_sel_win
    );
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
    mm_sel_win.load_currently_selected();
}


void MatchmakerLocationWindow::on_KEY_UP()
{
    mm_sel_win.select_previous();
}


void MatchmakerLocationWindow::on_KEY_DOWN()
{
    mm_sel_win.select_next();
}
