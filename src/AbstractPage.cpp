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



#include "AbstractPage.h"

#include "AbstractWindow.h"
#include "VisibilityAspect.h"
#include "key_codes.h"



AbstractPage::AbstractPage(std::vector<AbstractWindow *> w)
    : wins{w}
{
    for (auto win : wins)
        win->add_page(this);
}


AbstractPage::~AbstractPage()
{
    wins.clear();
    active_page() = nullptr;
    left_neighbor = nullptr;
    right_neighbor = nullptr;
    active_win = nullptr;
    prev_active_win = nullptr;
}


void AbstractPage::resize()
{
    if (is_active())
        for (auto w : wins)
            w->resize();
}


void AbstractPage::draw(bool clear_first)
{
    if (is_active())
        for (auto w : wins)
            w->draw(clear_first);
}


void AbstractPage::set_active_page(AbstractPage * pg)
{
    if (nullptr != active_page())
        for (auto w : active_page()->wins)
            w->disable(VisibilityAspect::PageVisibility::grab());

    active_page() = pg;

    if (nullptr != active_page())
        for (auto w : active_page()->wins)
            w->enable(VisibilityAspect::PageVisibility::grab());
}


void AbstractPage::set_active_window(AbstractWindow * act_win)
{
    if (nullptr != active_win)
        active_win->mark_dirty();

    prev_active_win = active_win;
    active_win = act_win;

    if (nullptr != active_win)
        active_win->mark_dirty();
}


void AbstractPage::set_active_window_to_previous()
{
    {
        AbstractWindow * w = active_win;
        active_win = prev_active_win;
        prev_active_win = w;
    }

    if (nullptr != prev_active_win)
        prev_active_win->mark_dirty();

    if (nullptr != active_win)
        active_win->mark_dirty();
}


void AbstractPage::on_KEY(int key)
{
    if (!is_active())
        return;

    switch (key)
    {
        case SHIFT_LEFT  : on_SHIFT_LEFT();  return;
        case SHIFT_RIGHT : on_SHIFT_RIGHT(); return;
    }

    on_KEY_hook(key);
}


void AbstractPage::on_SHIFT_LEFT()
{
    if (nullptr != left_neighbor)
        AbstractPage::set_active_page(left_neighbor);
}


void AbstractPage::on_SHIFT_RIGHT()
{
    if (nullptr != right_neighbor)
        AbstractPage::set_active_page(right_neighbor);
}
