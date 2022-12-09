#include "MatchmakerSelectionWindow.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include <ncurses.h>

#include "Settings.h"
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
