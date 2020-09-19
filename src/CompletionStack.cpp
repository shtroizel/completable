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



#include "CompletionStack.h"

#include <queue>

#include <matchmaker/matchmaker.h>



void CompletionStack::push(int ch)
{
    if (completion_count >= CAPACITY)
        return;

    {
        // keep reference to previous top for prefix initialization
        completion const & prev_top = top();

        // grow
        ++completion_count;
        clear_top();

        top().prefix = prev_top.prefix;
    }
    top().prefix += ch;

    // get new completion
    matchmaker::complete(
        top().prefix,
        top().start,
        top().length
    );

    // if adding 'ch' would make an unknown word then ignore (undo) the push
    if (top().length == 0)
    {
        pop();
        return;
    }

    // calculate length completion
    top().length_completion.reserve(top().length);
    std::make_heap(top().length_completion.begin(), top().length_completion.end());
    for (int i = top().start; i < top().start + top().length; ++i)
    {
        top().length_completion.push_back(matchmaker::as_longest(i));
        std::push_heap(top().length_completion.begin(), top().length_completion.end());
    }
    std::sort_heap(top().length_completion.begin(), top().length_completion.end());

    // set selected entry to first for both normal completion and length completion
    // notice how for normal completion this is a matchmaker index while for length completion
    // this is the first element in the "length_completion" vector
    top().display_start = top().start;
    top().len_display_start = 0;
}


void CompletionStack::pop()
{
    if (completion_count > 1)
    {
        clear_top();
        --completion_count;
    }
}


void CompletionStack::clear_top()
{
    top().prefix.clear();
    top().start = 0;
    top().display_start = 0;
    top().length = 0;
    top().len_display_start = 0;
    top().length_completion.clear();
    top().syn_display_start = 0;
    top().ant_display_start = 0;

    if (completion_count == 1)
    {
        // use entire dictionary for completions

        top().length = matchmaker::size();

        std::priority_queue<int, std::vector<int>, std::greater<std::vector<int>::value_type>> q;
        for (int i = 0; i < matchmaker::size(); ++i)
            q.push(matchmaker::as_longest(i));
        while (!q.empty())
        {
            top().length_completion.push_back(q.top());
            q.pop();
        }
    }
}
