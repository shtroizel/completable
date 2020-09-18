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



class CompletionStack
{
    static int const CAPACITY = 108;

public:
    struct completion
    {
        // user input state
        std::string prefix;

        // index of first word in dictionary that starts with prefix
        int start{0};

        // number of words in the dictionary that start with prefix
        int length{0};

        // index of first word displayed in "Completion"
        int display_start{0};

        // first index of "length_completion" or first word displayed in "Length Completion"
        int len_display_start{0};

        // sorted but noncontiguous length indexes
        std::vector<int> length_completion;

        // index of first word displayed in "Synonyms"
        int syn_display_start{0};

        // index of first word displayed in "Antonyms"
        int ant_display_start{0};
    };

    CompletionStack() { clear_top(); }

    void push(int ch);
    void pop();
    int count() const { return completion_count; }
    completion const & top() const { return completions[completion_count - 1]; }
    completion & top() { return completions[completion_count - 1]; }


private:
    void clear_top();

    completion completions[CAPACITY];
    int completion_count{1};
};
