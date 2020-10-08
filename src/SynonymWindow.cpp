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



#include "SynonymWindow.h"

#include <ncurses.h>

#include "CompletionWindow.h"
#include "CompletionStack.h"
#include "InputWindow.h"
#include "LengthCompletionWindow.h"
#include "matchmaker.h"
#include "word_filter.h"



SynonymWindow::SynonymWindow(
    CompletionStack & cs,
    WordStack & ws,
    InputWindow & iw,
    CompletionWindow & cw,
    word_filter & wf
)
    : AbstractListWindow(cs, ws, iw, wf)
{
    cw.add_dirty_dependency(this);
}


std::string SynonymWindow::title()
{
    std::string t{"Synonyms ("};

    auto syn = get_words();
    t += std::to_string(syn.size());

    t += ")";

    return t;
}


void SynonymWindow::resize_hook()
{
    y = 3;
    x = root_x / 2;

    int combined_height = root_y - 5 - y;
    height = combined_height / 1.618;
    width = x + root_x % 2;
}


int & SynonymWindow::display_start()
{
    return cs.top().syn_display_start;
}


std::vector<int> const & SynonymWindow::unfiltered_words(int word) const
{
    return matchmaker::synonyms(word);
}
