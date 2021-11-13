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



#include "OrdinalSummationWindow.h"

#include <ncurses.h>

#include "AbstractTab.h"
#include "CompletionStack.h"
#include "EnablednessSetting.h"
#include "InputWindow.h"
#include "CompletionWindow.h"
#include "matchmaker.h"



OrdinalSummationWindow::OrdinalSummationWindow(
    CompletionStack & cs,
    WordStack & ws,
    InputWindow & iw,
    word_filter & f,
    CompletionWindow & cw
)
    : AbstractListWindow(cs, ws, iw, f)
    , completion_win(cw)
{
    completion_win.add_dirty_dependency(this);
}


std::string OrdinalSummationWindow::title()
{
    std::string t{"Ordinal Summation: "};

    auto const & completion = cs.top().standard_completion;
    if (completion.size() > 0)
        t += std::to_string(matchmaker::ordinal_summation(completion[cs.top().display_start]));
    else
        t += "0";

    t += "  (";
    auto os = get_words();
    t += std::to_string(os.size());
    t += ")";

    return t;
}


void OrdinalSummationWindow::resize_hook()
{
	y = completion_win.get_y() + completion_win.get_height();
    x = 0;
    height = root_y - y - 5;
    width = completion_win.get_width();
}


int & OrdinalSummationWindow::display_start()
{
    return cs.top().ord_sum_display_start;
}


void OrdinalSummationWindow::unfiltered_words(int index, int const * * words, int * count) const
{
    matchmaker::from_ordinal_summation(matchmaker::ordinal_summation(index), words, count);
}
