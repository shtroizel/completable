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


// ncurses window
typedef struct _win_st WINDOW;

class CompletionStack;


static int const PAGE_UP{339};
static int const PAGE_DOWN{338};
static int const HOME{262};
static int const END{360};


class AbstractWindow
{
public:
    AbstractWindow(AbstractWindow const &) = delete;
    AbstractWindow & operator=(AbstractWindow const &) = delete;

    AbstractWindow() {}
    virtual ~AbstractWindow();

    void clear();
    void resize();
    void draw(CompletionStack const & cs, bool clear_first);
    int get_height() const { return height; }
    int get_width() const { return width; }
    int get_y() const { return y; }
    int get_x() const { return x; }
    WINDOW * get_WINDOW() const { return w; }

    static void set_active_window(AbstractWindow * act_win) { active() = act_win; }
    static AbstractWindow * get_active_window() { return active(); }

    bool is_active() const { return nullptr != active() && active()->title() == title(); }

    void on_KEY(int key, CompletionStack & cs);

private:
    virtual std::string const & title() const = 0;
    virtual void resize_hook() = 0;
    virtual void post_resize_hook() {};
    virtual void draw_hook(CompletionStack const & cs) = 0;
    virtual void on_KEY_UP(CompletionStack &) {}
    virtual void on_KEY_DOWN(CompletionStack &) {}
    virtual void on_PAGE_UP(CompletionStack &) {}
    virtual void on_PAGE_DOWN(CompletionStack &) {}
    virtual void on_HOME(CompletionStack &) {}
    virtual void on_END(CompletionStack &) {}

protected:
    WINDOW * w{nullptr};
    int root_y{0};
    int root_x{0};
    int height{0};
    int width{0};
    int y{0};
    int x{0};

    // static variable for storing the active window
    static AbstractWindow * & active() { static AbstractWindow * w{nullptr}; return w; }
};
