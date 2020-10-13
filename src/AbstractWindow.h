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


#include <memory>
#include <stack>
#include <string>
#include <vector>

#include <matchable/matchable_fwd.h>

#include "word_stack_element.h"


// ncurses window
typedef struct _win_st WINDOW;

class AbstractPage;
class CompletionStack;
using WordStack = std::stack<word_stack_element>;
MATCHABLE_FWD(VisibilityAspect);
MATCHABLE_FWD(Layer);

class AbstractWindow
{
public:
    AbstractWindow(AbstractWindow const &) = delete;
    AbstractWindow & operator=(AbstractWindow const &) = delete;

    AbstractWindow();
    virtual ~AbstractWindow();

    void clear();
    void resize();
    void draw(bool clear_first);
    Layer::Type get_layer() const;
    int get_height() const { return height; }
    int get_width() const { return width; }
    int get_y() const { return y; }
    int get_x() const { return x; }
    WINDOW * get_WINDOW() const { return w; }
    std::string get_title() { return title(); }

    void set_left_neighbor(AbstractWindow * neighbor);
    void set_right_neighbor(AbstractWindow * neighbor);
    AbstractWindow * get_left_neighbor() { return left_neighbor; };
    AbstractWindow * get_right_neighbor() { return right_neighbor; };

    bool is_active();

    bool belongs_to_active_page();
    void add_page(AbstractPage *);

    void on_KEY(int key);

    void mark_dirty();
    void add_dirty_dependency(AbstractWindow * win);

    void set_enabled(bool, VisibilityAspect::Type);
    bool is_enabled() const;
    bool is_enabled(VisibilityAspect::Type) const;
    void enable(VisibilityAspect::Type);
    void disable(VisibilityAspect::Type);

    void activate_left();
    void activate_right();

private:
    // dependencies
    virtual std::string title() = 0;
    virtual void resize_hook() = 0;
    virtual void draw_hook() = 0;
    virtual Layer::Type layer() const = 0;

    // options
    virtual bool borders_enabled() const { return true; }
    virtual void post_resize_hook() {};
    virtual void pre_disable_hook() {}
    virtual void on_KEY_UP() {}
    virtual void on_KEY_DOWN() {}
    virtual void on_PAGE_UP() {}
    virtual void on_PAGE_DOWN() {}
    virtual void on_HOME() {}
    virtual void on_END() {}
    virtual void on_RETURN() {}
    virtual void on_DELETE() {}
    virtual void on_TAB() {}
    virtual void on_BACKSPACE() {}
    virtual void on_printable_ascii(int) {}


protected:
    WINDOW * w{nullptr};
    int root_y{0};
    int root_x{0};
    int height{0};
    int width{0};
    int y{0};
    int x{0};
    std::vector<AbstractPage *> pages;

private:
    std::shared_ptr<VisibilityAspect::Flags> disabled;
    bool dirty{false};
    std::vector<AbstractWindow *> dirty_dependencies;
    AbstractWindow * left_neighbor{nullptr};
    AbstractWindow * right_neighbor{nullptr};
};
