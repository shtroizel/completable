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



#include "PartsOfSpeechWindow.h"

#include <ncurses.h>

#include <matchmaker/matchmaker.h>

#include "CompletionStack.h"



std::string const & PartsOfSpeechWindow::title() const
{
    static std::string const t{"Parts of Speech"};
    return t;
}


void PartsOfSpeechWindow::resize_hook()
{
    height = 5;
    width = root_x;
    y = root_y - height;
    x = 0;
}


void PartsOfSpeechWindow::draw_hook()
{
    int const cell_width{16};

    auto const & flagged_pos = matchmaker::flagged_parts_of_speech(cs.top().display_start);

    int x = 1;
    int i = 0;
    int y = 1;
    while (i < (int) matchmaker::all_parts_of_speech().size())
    {
        if (flagged_pos[i] != 0)
            wattron(w, A_REVERSE);

        for(
            int j = 0;
            j < (int) matchmaker::all_parts_of_speech()[i].size() && j < x + cell_width - 2;
            ++j
        )
            mvwaddch(w, y, j + x, matchmaker::all_parts_of_speech()[i][j]);

        if (flagged_pos[i] != 0)
            wattroff(w, A_REVERSE);

        ++i;

        x += cell_width;

        if (x > cell_width * 5)
        {
            if (y < 4)
            {
                y += 1;
                x = 1;
            }
            else
            {
                break;
            }
        }
    }
}
