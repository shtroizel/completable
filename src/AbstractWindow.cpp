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



#include "AbstractWindow.h"

#include <algorithm>

#include <ncurses.h>

#include "AbstractTab.h"
#include "CompletionStack.h"
#include "EnablednessSetting.h"
#include "Layer.h"
#include "VisibilityAspect.h"
#include "key_codes.h"



// minimum required terminal height
static int const MIN_ROOT_Y{27};

// minimum required terminal width
static int const MIN_ROOT_X{80};



AbstractWindow::AbstractWindow() : disabled{std::make_shared<VisibilityAspect::Flags>()}
{
}


AbstractWindow::~AbstractWindow()
{
    delwin(w);
    w = nullptr;
}


void AbstractWindow::clear()
{
    if (nullptr != w)
    {
        wclear(w);
        wrefresh(w);
    }
}


void AbstractWindow::resize()
{
    if (!is_enabled())
        return;

    if (nullptr != w)
    {
        clear();
        delwin(w);
        w = nullptr;
    }

    getmaxyx(stdscr, root_y, root_x);
    resize_hook();
    w = newwin(height, width, y, x);
    post_resize_hook();
    mark_dirty();
}


void AbstractWindow::draw(bool clear_first)
{
    if (!is_enabled())
        return;

    // check terminal for minimum size requirement
    if (root_y < MIN_ROOT_Y || root_x < MIN_ROOT_X)
    {
        wclear(w);
        wrefresh(w);
        return;
    }

    // always draw when clear_first regardless of dirty
    if (!dirty && !clear_first)
        return;

    if (clear_first)
        wclear(w);

    // border
    if (EnablednessSetting::Borders::grab().as_enabledness() == Enabledness::Enabled::grab() &&
            borders_enabled())
        box(w, 0, 0);

    // title
    {
        int indent = width - (int) ((width / 1.618 + title().length() / 2.0) + 0.5);
        mvwaddch(w, 0, indent - 1, ' ');
        {
            int const active_indicator_left = is_active() ? '>' : ' ';
            int const active_indicator_right = is_active() ? '<' : ' ';
            mvwaddch(w, 0, indent - 2, active_indicator_left);
            mvwaddch(w, 0, title().size() + indent + 1, active_indicator_right);
        }
        for (int i = 0; i < (int) title().size(); ++i)
            mvwaddch(w, 0, i + indent, title()[i]);
        mvwaddch(w, 0, title().size() + indent, ' ');
    }

    // window specific drawing
    draw_hook();

    dirty = false;

    wrefresh(w);
}


Layer::Type AbstractWindow::get_layer() const
{
    return layer();
}


void AbstractWindow::set_left_neighbor(AbstractWindow * n)
{
    if (nullptr == n)
        return;

    if (layer() != n->layer())
        return;

    left_neighbor = n;
}


void AbstractWindow::set_right_neighbor(AbstractWindow * n)
{
    if (nullptr == n)
        return;

    if (layer() != n->layer())
        return;

    right_neighbor = n;
}


bool AbstractWindow::is_active()
{
    if (!belongs_to_active_tab())
        return false;

    auto active_tab = AbstractTab::get_active_tab();

    auto act_win = active_tab->get_active_window();
    if (nullptr == act_win)
        return false;

    return act_win == this;
}


bool AbstractWindow::belongs_to_active_tab()
{
    auto active_tab = AbstractTab::get_active_tab();
    if (nullptr == active_tab)
        return false;

    for (auto tab : tabs)
        if (tab == active_tab)
            return true;

    return false;
}


void AbstractWindow::add_tab(AbstractTab * tab)
{
    tabs.push_back(tab);
}


void AbstractWindow::on_KEY(int key)
{
    if (!is_enabled())
        return;

    if (!is_active())
        return;

    switch (key)
    {
        case KEY_LEFT           : activate_left();  return;
        case KEY_RIGHT          : activate_right(); return;
        case KEY_UP             : on_KEY_UP();      return;
        case KEY_DOWN           : on_KEY_DOWN();    return;
        case PAGE_UP            : on_PAGE_UP();     return;
        case PAGE_DOWN          : on_PAGE_DOWN();   return;
        case HOME               : on_HOME();        return;
        case END                : on_END();         return;
        case RETURN             : on_RETURN();      return;
        case DELETE             : on_DELETE();      return;
        case TAB                : on_TAB();         return;
        case KEY_BACKSPACE      :
        case BACKSPACE_127      :
        case BACKSPACE_BKSLSH_B : on_BACKSPACE(); return;
    }

    if (key > 31 && key < 127) // printable ascii
        on_printable_ascii(key);
}


void AbstractWindow::mark_dirty()
{
    dirty = true;
    for (auto dep : dirty_dependencies)
        dep->mark_dirty();
}


void AbstractWindow::add_dirty_dependency(AbstractWindow * dep)
{
    dirty_dependencies.push_back(dep);
}


void AbstractWindow::set_enabled(bool state, VisibilityAspect::Type aspect)
{
    if (state)
        enable(aspect);
    else
        disable(aspect);
}


bool AbstractWindow::is_enabled() const
{
    return 0 == disabled->currently_set().size();
}


bool AbstractWindow::is_enabled(VisibilityAspect::Type aspect) const
{
    return !disabled->is_set(aspect);
}


void AbstractWindow::enable(VisibilityAspect::Type aspect)
{
    disabled->unset(aspect);
    if (is_enabled())
        resize();
}


void AbstractWindow::disable(VisibilityAspect::Type aspect)
{
    if (is_enabled())
    {
        pre_disable_hook();

        if (nullptr != w)
        {
            clear();
            delwin(w);
            w = nullptr;
        }
    }

    disabled->set(aspect);
    mark_dirty();
}


void AbstractWindow::activate_left()
{
    if (nullptr == left_neighbor)
        return;

    auto active_tab = AbstractTab::get_active_tab();
    if (nullptr == active_tab)
        return;

    AbstractWindow * l = left_neighbor;
    while (!l->is_enabled(VisibilityAspect::WindowVisibility::grab()) && l != this)
        l = l->get_left_neighbor();

    if (l != this && l->is_enabled(VisibilityAspect::WindowVisibility::grab()))
        active_tab->set_active_window(l);
}


void AbstractWindow::activate_right()
{
    if (nullptr == left_neighbor)
        return;

    auto active_tab = AbstractTab::get_active_tab();
    if (nullptr == active_tab)
        return;

    AbstractWindow * r = right_neighbor;
    while (!r->is_enabled(VisibilityAspect::WindowVisibility::grab()) && r != this)
        r = r->get_right_neighbor();

    if (r != this && r->is_enabled(VisibilityAspect::WindowVisibility::grab()))
        active_tab->set_active_window(r);
}
