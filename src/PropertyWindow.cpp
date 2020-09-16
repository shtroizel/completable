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



#include "PropertyWindow.h"

#include <matchmaker/matchmaker.h>

#include "CompletionStack.h"
#include "CompletionWindow.h"
#include "LengthCompletionWindow.h"


PropertyWindow::PropertyWindow(CompletionWindow & cw, LengthCompletionWindow & lcw)
    : completion_win(cw), len_completion_win(lcw)
{
    completion_win.add_dirty_dependency(this);
    len_completion_win.add_dirty_dependency(this);
}


PropertyWindow::~PropertyWindow()
{
    completion_win.remove_dirty_dependency(this);
    len_completion_win.remove_dirty_dependency(this);
}


void PropertyWindow::draw_hook(CompletionStack const & cs)
{
    int selected{0};
    if (completion_win.is_active())
    {
        selected = cs.top().display_start;
    }
    else if (len_completion_win.is_active())
    {
        int length_completion_index = cs.top().len_display_start;
        if (length_completion_index >= (int) cs.top().length_completion.size())
            return;
        if (length_completion_index < 0)
            return;
        int long_index = cs.top().length_completion[length_completion_index];
        selected = matchmaker::from_longest(long_index);
    }
    else
    {
        return;
    }

    draw_hook(selected);
}
