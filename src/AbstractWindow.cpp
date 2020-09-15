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

#include "CompletionStack.h"


// minimum required terminal height
static int const MIN_ROOT_Y{30};

// minimum required terminal width
static int const MIN_ROOT_X{80};


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
    keypad(w, true);
}


void AbstractWindow::draw(CompletionStack const & cs)
{
    wclear(w);

    // check terminal for minimum size requirement
    if (root_y < MIN_ROOT_Y || root_x < MIN_ROOT_X)
    {
        wrefresh(w);
        return;
    }

    // border
    box(w, 0, 0);

    // title
    int const indent{width / 3 - (int) title().size() / 2};
    mvwaddch(w, 0, indent - 1, ' ');
    for (int i = 0; i < (int) title().size(); ++i)
        mvwaddch(w, 0, i + indent, title()[i]);
    mvwaddch(w, 0, title().size() + indent, ' ');

    // window specific drawing
    draw_hook(cs);

    wrefresh(w);
}
