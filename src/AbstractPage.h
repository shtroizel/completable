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


#include <array>
#include <memory>
#include <vector>

#include <matchable/matchable_fwd.h>



MATCHABLE_FWD(Layer)

class AbstractWindow;

class AbstractPage
{
public:
    AbstractPage(AbstractPage const &) = delete;
    AbstractPage & operator=(AbstractPage const &) = delete;

    AbstractPage();
    virtual ~AbstractPage();

    void add_window(AbstractWindow *);

    void resize();
    void draw(bool clear_first);

    void set_left_neighbor(AbstractPage * neighbor) { left_neighbor = neighbor; }
    void set_right_neighbor(AbstractPage * neighbor) { right_neighbor = neighbor; }

    static void set_active_page(AbstractPage *);
    static AbstractPage * get_active_page() { return active_page(); }
    bool is_active() { return nullptr != active_page() && active_page()->description() == description(); }

    void set_active_window(AbstractWindow *);
    AbstractWindow * get_active_window();
    AbstractWindow * get_active_window(Layer::Type);

    void on_KEY(int key);

    std::array<char, 17> const & get_description() const { return description(); }
    char get_abbreviation() const { return abbreviation(); }
    int get_indicator_position() const { return indicator_position(); }


private:
    // dependencies
    virtual std::array<char, 17> const & description() const = 0;
    virtual char abbreviation() const = 0;
    virtual int indicator_position() const = 0;

    void on_ANY_F();
    void on_COMMA();
    void toggle_layer(Layer::Type, bool &);
    void on_SHIFT_LEFT();
    void on_SHIFT_RIGHT();


private:
    AbstractPage * left_neighbor{nullptr};
    AbstractPage * right_neighbor{nullptr};

    std::shared_ptr<
        matchable::MatchBox<
            Layer::Type,
            std::pair<std::vector<AbstractWindow *>, AbstractWindow *>
        >
    > content;
    bool layer_F_enabled{false};
    bool layer_Help_enabled{false};

    // static variables that look like functions
    static AbstractPage * & active_page() { static AbstractPage * w{nullptr}; return w; }
};
