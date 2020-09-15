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



#include "AntonymWindow.h"

#include <ncurses.h>

#include <matchmaker/matchmaker.h>

#include "LengthCompletionWindow.h"



std::string const & AntonymWindow::title() const
{
    static std::string const t{"Antonyms"};
    return t;
}


void AntonymWindow::resize_hook()
{
    height = len_completion_win.get_height();
    width = len_completion_win.get_width() + root_x % 2;
    y = len_completion_win.get_y();
    x = len_completion_win.get_width();
}


void AntonymWindow::draw_hook(int selected)
{
    auto const & antonyms = matchmaker::antonyms(selected);
    for (int i = 0; i < (int) antonyms.size() && i < height - 2; ++i)
    {
        std::string const & ant = matchmaker::at(antonyms[i]);
        for (int j = 0; j < (int) ant.length() && j < width - 2; ++j)
        {
            mvwaddch(
                w,
                i + 1,
                j + 1,
                ant[j]
            );
        }
    }
}
