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



#include "LengthCompletionWindow.h"

#include <ncurses.h>

#include "CompletionStack.h"
#include "CompletionWindow.h"
#include "InputWindow.h"
#include "matchmaker.h"
#include "word_filter.h"



LengthCompletionWindow::LengthCompletionWindow(
    CompletionStack & cs,
    WordStack & ws,
    InputWindow & iw,
    word_filter & wf,
    CompletionWindow const & cw
)
    : AbstractListWindow(cs, ws, iw, wf)
    , completion_win(cw)
{
}


std::string LengthCompletionWindow::title()
{
    static std::string const t{"Length Completion"};
    return t;
}


void LengthCompletionWindow::resize_hook()
{
    y = completion_win.get_y() + completion_win.get_height();
    x = 0;
    height = root_y - y - 5;
    width = completion_win.get_width();
}


int & LengthCompletionWindow::display_start()
{
    return cs.top().len_display_start;
}


void LengthCompletionWindow::unfiltered_words(int, int const * * words, int * count) const
{
    *words = cs.top().length_completion.data();
    *count = (int) cs.top().length_completion.size();
}


char const * LengthCompletionWindow::string_from_index(int index, int * len)
{
    return matchmaker::at(matchmaker::from_longest(index), len);
}
