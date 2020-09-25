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



#include "AbstractWindow.h"



class InputWindow;
struct word_filter;

class AbstractSynAntWindow : public AbstractWindow
{
public:
    AbstractSynAntWindow(CompletionStack &, WordStack &, InputWindow &, word_filter &);

protected:
    void filtered_words(std::vector<int> & filtered);

private:
    // resolved dependencies
    void draw_hook() override;
    void on_KEY_UP() override;
    void on_KEY_DOWN() override;
    void on_PAGE_UP() override;
    void on_PAGE_DOWN() override;
    void on_HOME() override;
    void on_END() override;
    void on_RETURN() override;

    // new dependencies
    virtual int & display_start() = 0;
    virtual std::vector<int> const & unfiltered_words(int) = 0;

    // new options
    virtual void on_post_RETURN() {}

    // cache
    std::vector<int> words;
    class CacheDirty
    {
    public:
        void set_false() { dirty = false; }
        bool is_dirty_and_set_true() { bool d = dirty; dirty = true; return d; }
    private:
        bool dirty{false};
    };
    CacheDirty cache_dirty;

    // same or longer lifetime as this and avoided during destruction
    InputWindow & input_win;
    word_filter & wf;
};