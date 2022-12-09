#include "SettingsWindow.h"

#include <iostream>
#include <string>
#include <vector>

#include <ncurses.h>

#include "InputWindow.h"
#include "Layer.h"
#include "Settings.h"
#include "VisibilityAspect.h"



std::string SettingsWindow::title()
{
    static std::string const t{"Settings"};
    return t;
}


void SettingsWindow::resize_hook()
{
    height = EnablednessSetting::variants().size() + 4;
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
            for (auto s : AnimationSetting::variants())
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
            "%s", setting.as_string().c_str()
        );

        mvwprintw(w, setting.as_index() + 2, right_align, ":");

        // print enabledness
        if (setting.as_index() == selection)
            wattron(w, A_REVERSE);
        mvwprintw(w, setting.as_index() + 2, right_align + 2,
                  "%s", setting.as_enabledness().as_string().c_str());
        wattroff(w, A_REVERSE);

        // clear space after "Enabled" since "Disabled" is a char longer
        mvwaddch(
            w,
            setting.as_index() + 2,
            right_align + 2 + setting.as_enabledness().as_string().size(),
            ' '
        );
    }

    for (auto setting : AnimationSetting::variants())
    {
        // print setting name
        mvwprintw(
            w,
            EnablednessSetting::variants().size() + setting.as_index() + 2,
            right_align - setting.as_string().length(),
            "%s", setting.as_string().c_str()
        );

        mvwprintw(w, EnablednessSetting::variants().size() + setting.as_index() + 2, right_align, ":");

        // print enabledness
        if (setting.as_index() + (int) EnablednessSetting::variants().size() == selection)
            wattron(w, A_REVERSE);
        mvwprintw(
            w,
            EnablednessSetting::variants().size() + setting.as_index() + 2,
            right_align + 2,
            "%s", setting.as_animation().as_string().c_str()
        );
        wattroff(w, A_REVERSE);

        // clear space after "Enabled" since "Disabled" is a char longer
        mvwprintw(
            w,
            EnablednessSetting::variants().size() + setting.as_index() + 2,
            right_align + 2 + setting.as_animation().as_string().size(),
            "      "
        );
    }
}


Layer::Type SettingsWindow::layer() const
{
    return Layer::Bottom::grab();
}


void SettingsWindow::on_RETURN()
{
    if (selection < (int) EnablednessSetting::variants().size())
    {
        auto setting = EnablednessSetting::from_index(selection);

        int index = setting.as_enabledness().as_index() + 1;
        if (index >= (int) Enabledness::variants().size())
            index = 0;

        setting.set_enabledness(Enabledness::from_index(index));
        mark_dirty();
    }
    else if (selection < (int)(EnablednessSetting::variants().size() + AnimationSetting::variants().size()))
    {
        auto setting = AnimationSetting::from_index(selection - EnablednessSetting::variants().size());

        int index = setting.as_animation().as_index() + 1;
        if (index >= (int) Animation::variants().size())
            index = 0;

        setting.set_animation(Animation::from_index(index));
        mark_dirty();
    }
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
    if (selection < (int) (EnablednessSetting::variants().size() + AnimationSetting::variants().size() - 1))
    {
        ++selection;
        mark_dirty();
    }
}
