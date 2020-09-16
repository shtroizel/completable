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



AbstractWindow::~AbstractWindow()
{
    set_active_window(nullptr);
    delwin(w);
}


void AbstractWindow::clear()
{
    wclear(w);
    wrefresh(w);
}


void AbstractWindow::resize()
{
    if (nullptr != w)
    {
        clear();
        delwin(w);
    }

    getmaxyx(stdscr, root_y, root_x);
    resize_hook();
    w = newwin(height, width, y, x);
    post_resize_hook();
    mark_dirty();
}


void AbstractWindow::draw(CompletionStack const & cs, bool clear_first)
{
    // check terminal for minimum size requirement
    if (root_y < MIN_ROOT_Y || root_x < MIN_ROOT_X)
    {
        wclear(w);
        wrefresh(w);
        return;
    }

    if (!dirty && !clear_first)
        return;

    if (clear_first)
        wclear(w);

    // border
    box(w, 0, 0);

    // title
    int const indent{width / 3 - (int) title().size() / 2};
    mvwaddch(w, 0, indent - 1, ' ');
    if (is_active())
    {
        mvwaddch(w, 0, indent - 2, '>');
        mvwaddch(w, 0, title().size() + indent + 1, '<');
    }
    for (int i = 0; i < (int) title().size(); ++i)
        mvwaddch(w, 0, i + indent, title()[i]);
    mvwaddch(w, 0, title().size() + indent, ' ');

    // window specific drawing
    draw_hook(cs);

    dirty = false;

    wrefresh(w);
}


void AbstractWindow::set_active_window(AbstractWindow * act_win)
{
    if (active() != nullptr)
        active()->mark_dirty();

    active() = act_win;

    if (active() != nullptr)
        active()->mark_dirty();
}


void AbstractWindow::on_KEY(int key, CompletionStack & cs)
{
    switch (key)
    {
        case KEY_UP    : on_KEY_UP(cs);    break;
        case KEY_DOWN  : on_KEY_DOWN(cs);  break;
        case PAGE_UP   : on_PAGE_UP(cs);   break;
        case PAGE_DOWN : on_PAGE_DOWN(cs); break;
        case HOME      : on_HOME(cs);      break;
        case END       : on_END(cs);       break;
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


void AbstractWindow::remove_dirty_dependency(AbstractWindow * dep)
{
    dirty_dependencies.erase(
        std::remove_if(
            dirty_dependencies.begin(),
            dirty_dependencies.end(),
            [&](AbstractWindow * w){ return w->title() == dep->title(); }
        ),
        dirty_dependencies.end()
    );
}
