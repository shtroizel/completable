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



#include "AbstractCompletionDataWindow.h"



class InputWindow;
struct word_filter;



/**
 * The AbstractListWindow class provides drawing and event handling for windows that need to
 * display lists of words based on "completion data"
 */
class AbstractListWindow : public AbstractCompletionDataWindow
{
public:
    AbstractListWindow(CompletionStack &, WordStack &, InputWindow &, word_filter &);

protected:
    std::vector<int> const & get_words() const;

private:
    // resolved dependencies
    void draw_hook() final;
    Layer::Type layer() const final;

    // options
    void on_KEY_UP() final;
    void on_KEY_DOWN() final;
    void on_PAGE_UP() final;
    void on_PAGE_DOWN() final;
    void on_HOME() final;
    void on_END() final;
    void on_RETURN() final;
    void on_DELETE() final;
    void on_TAB() final;
    void on_BACKSPACE() final;
    void on_printable_ascii(int) final;

    // new dependencies
    virtual int & display_start() = 0;
    virtual void unfiltered_words(int index, int const * * words, int * count) const = 0;
    virtual bool apply_filter() const = 0;

    // new options
    virtual void on_post_RETURN() {}
    virtual char const * string_from_index(int index, int * len);

    // cache
    std::vector<int> words;
    class CacheDirty // because observing is disturbing (see is_dirty())
    {
    public:
        void set_false() { dirty = false; }
        bool is_dirty() const { prev_dirty = dirty; dirty = true; return prev_dirty; }
    private:
        mutable bool dirty{true};
        mutable bool prev_dirty{true};
    };
    CacheDirty cache_dirty;
    mutable std::vector<int> words_cache;

    InputWindow & input_win;
    word_filter & wf;
};
