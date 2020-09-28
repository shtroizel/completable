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

#include "CompletionStack.h"



// minimum required terminal height
static int const MIN_ROOT_Y{30};

// minimum required terminal width
static int const MIN_ROOT_X{80};


AbstractWindow::AbstractWindow(CompletionStack & completion_stack, WordStack & word_stack)
    : cs{completion_stack}, ws{word_stack}
{
}


AbstractWindow::~AbstractWindow()
{
    set_active_window(nullptr);
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
    if (!enabled)
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
    if (!enabled)
        return;

    // check terminal for minimum size requirement
    if (root_y < MIN_ROOT_Y || root_x < MIN_ROOT_X)
    {
        wclear(w);
        wrefresh(w);
        return;
    }

    // always draw when clear_first
    if (!dirty && !clear_first)
        return;

    if (clear_first)
        wclear(w);

    // border
    if (borders())
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


void AbstractWindow::set_active_window(AbstractWindow * act_win)
{
    if (nullptr != active())
        active()->mark_dirty();

    prev_active() = active();
    active() = act_win;

    if (nullptr != active())
        active()->mark_dirty();
}


void AbstractWindow::set_active_window_to_previous()
{
    AbstractWindow * w = active();
    active() = prev_active();
    prev_active() = w;

    if (nullptr != prev_active())
        prev_active()->mark_dirty();

    if (nullptr != active())
        active()->mark_dirty();
}


void AbstractWindow::on_KEY(int key)
{
    if (!enabled)
        return;

    if (!is_active())
        return;

    switch (key)
    {
        case KEY_UP    : on_KEY_UP();    return;
        case KEY_DOWN  : on_KEY_DOWN();  return;
        case PAGE_UP   : on_PAGE_UP();   return;
        case PAGE_DOWN : on_PAGE_DOWN(); return;
        case HOME      : on_HOME();      return;
        case END       : on_END();       return;
        case RETURN    : on_RETURN();    return;
        case DELETE    : on_DELETE();    return;
        case KEY_LEFT  : on_KEY_LEFT();  return;
        case KEY_RIGHT : on_KEY_RIGHT(); return;
    }
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


void AbstractWindow::enable()
{
    enabled = true;
    resize();
}


void AbstractWindow::disable()
{
    pre_disable_hook();

    if (nullptr != w)
    {
        clear();
        delwin(w);
        w = nullptr;
    }

    enabled = false;
}


void AbstractWindow::on_KEY_LEFT()
{
    if (nullptr != left_neighbor)
        AbstractWindow::set_active_window(left_neighbor);
}


void AbstractWindow::on_KEY_RIGHT()
{
    if (nullptr != right_neighbor)
        AbstractWindow::set_active_window(right_neighbor);
}
