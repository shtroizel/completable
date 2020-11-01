#pragma once

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


#include <string>
#include <vector>

#include <matchable/matchable_fwd.h>


struct word_filter;


/**
 * The CompletionStack class provides the data for everything that is seen in the various views under
 * the "completable" tab. The stack consists of "completion" structs, with each representing the letters
 * making up a word as typed so far (completion.prefix).
 */
class CompletionStack
{
    static int const CAPACITY = 108;

public:
    struct completion
    {
        // user input state
        std::string prefix;

        std::vector<int> standard_completion;

        // first index displayed for standard_completion
        int display_start{0};

        std::vector<int> length_completion;

        // first index displayed for length_completion
        int len_display_start{0};

        // first synonym displayed
        int syn_display_start{0};

        // first antonym displayed
        int ant_display_start{0};
    };

    CompletionStack(word_filter const &);

    /**
     * Add a letter (character) to the stack if doing so would make an known word
     * @param[in] ch Character to push onto the stack
     */
    void push(int ch);

    /**
     * Removes the last letter (character) if any present
     */
    void pop();

    /**
     * The stack's count is always at least 1, since an empty prefix is a completion with all words.
     * Generally, the stack's count is always one more than top().prefix.length()
     *
     * @returns The number of elements (completions) within the stack
     */
    int count() const { return completion_count; }

    /**
     * @returns the latest completion or top of the stack
     */
    completion const & top() const { return completions[completion_count - 1]; }
    completion & top() { return completions[completion_count - 1]; }

    /**
     * clear the completion data for the latest completion
     */
    void clear_top();

    /**
     * clear the completion data for all completions
     */
    void clear_all();


private:
    completion completions[CAPACITY];
    int completion_count{1};

    word_filter const & wf;
};
