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

#include "AbstractPage.h"
#include "CompletionStack.h"
#include "InputWindow.h"
#include "CompletionWindow.h"
#include "LengthCompletionWindow.h"
#include "SynonymWindow.h"
#include "matchmaker.h"



AntonymWindow::AntonymWindow(
    CompletionStack & cs,
    WordStack & ws,
    InputWindow & iw,
    CompletionWindow & cw,
    LengthCompletionWindow & lcw,
    SynonymWindow & sw,
    word_filter & f
)
    : AbstractListWindow(cs, ws, iw, f)
    , len_completion_win(lcw)
    , syn_win(sw)
{
    cw.add_dirty_dependency(this);
}


std::string AntonymWindow::title()
{
    std::string t{"Antonyms ("};

    auto ant = get_words();
    t += std::to_string(ant.size());

    t += ")";

    return t;
}


void AntonymWindow::resize_hook()
{
    height = len_completion_win.get_height();
    width = len_completion_win.get_width() + root_x % 2;
    y = len_completion_win.get_y();
    x = len_completion_win.get_width();
}


int & AntonymWindow::display_start()
{
    return cs.top().ant_display_start;
}


void AntonymWindow::on_post_RETURN()
{
    AbstractPage::get_active_page()->set_active_window(&syn_win);
}


std::vector<int> const & AntonymWindow::unfiltered_words(int word) const
{
    return matchmaker::antonyms(word);
}
