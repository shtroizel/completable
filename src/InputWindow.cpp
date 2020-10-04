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



#include "InputWindow.h"

#include <ncurses.h>

#include "CompletionStack.h"
#include "Layer.h"



std::string InputWindow::title()
{
    static std::string const t{"enter , for help"};
    return t;
}


void InputWindow::resize_hook()
{
    height = 3;
    width = root_x / 2;
    y = 0;
    x = root_x / 2 - width / 2;
}


void InputWindow::draw_hook()
{
    std::string const & prefix = cs.top().prefix;

    int x = 0;
    for (; x < width - 2 && x < (int) prefix.size(); ++x)
        mvwaddch(w, 1, x + 1, prefix[x]);

    // blank out rest of line
    for (; x < width - 2; ++x)
        mvwaddch(w, 1, x + 1, ' ');
}


Layer::Type InputWindow::layer() const
{
    return Layer::Bottom::grab();
}
