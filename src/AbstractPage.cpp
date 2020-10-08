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

#include <ncurses.h>

#include "AbstractWindow.h"
#include "Layer.h"
#include "VisibilityAspect.h"
#include "key_codes.h"



AbstractPage::AbstractPage()
    : content{
        std::make_shared<
            matchable::MatchBox<
                Layer::Type,
                std::pair<std::vector<AbstractWindow *>, AbstractWindow *>
            >
        >()
      }
{
    for (auto l : Layer::variants())
        content->mut_at(l).second = nullptr;
}


AbstractPage::~AbstractPage()
{
    content.reset();
    active_page() = nullptr;
    left_neighbor = nullptr;
    right_neighbor = nullptr;
}


void AbstractPage::add_window(AbstractWindow * win)
{
    if (nullptr == win)
        return;

    content->mut_at(win->get_layer()).first.push_back(win);
    win->add_page(this);

    // guarantee active window by setting first window active
    if (content->at(win->get_layer()).first.size() == 1)
        content->mut_at(win->get_layer()).second = win;
}


void AbstractPage::resize()
{
    if (is_active())
        for (auto l : Layer::variants())
            for (auto w : content->mut_at(l).first)
                w->resize();
}


void AbstractPage::draw(bool clear_first)
{
    if (is_active())
    {
        for (auto w : content->mut_at(Layer::Bottom::grab()).first)
            w->draw(clear_first);

        if (layer_F_enabled)
            for (auto w : content->mut_at(Layer::F::grab()).first)
                w->draw(clear_first);

        if (layer_Help_enabled)
            for (auto w : content->mut_at(Layer::Help::grab()).first)
                w->draw(clear_first);
    }
}



void AbstractPage::set_active_page(AbstractPage * pg)
{
    if (nullptr == pg)
        return;

    if (nullptr != active_page())
        for (auto l : Layer::variants())
            for (auto w : active_page()->content->mut_at(l).first)
                w->disable(VisibilityAspect::PageVisibility::grab());

    // synce WindowVisibility aspect for Help layer, specifically:
    //
    // if help enabled for old page then enable help for new page if not already
    // and...
    // if help disabled for old page then disable help for new page if not already
    if (nullptr != active_page())
    {
        if (active_page()->get_active_window()->get_layer() == Layer::Help::grab())
        {
            if (pg->get_active_window()->get_layer() != Layer::Help::grab())
                pg->on_COMMA();
        }
        else
        {
            if (pg->get_active_window()->get_layer() == Layer::Help::grab())
                pg->on_COMMA();
        }
    }

    active_page() = pg;

    if (nullptr != active_page())
        for (auto l : Layer::variants())
            for (auto w : active_page()->content->mut_at(l).first)
                w->enable(VisibilityAspect::PageVisibility::grab());
}


void AbstractPage::set_active_window(AbstractWindow * win)
{
    if (nullptr == win)
        return;

    // verify given window exists in the given layer
    {
        bool found = false;
        for (auto w : content->at(win->get_layer()).first)
        {
            if (w == win)
            {
                found = true;
                break;
            }
        }
        if (!found)
            return;
    }

    // need to redraw both old and new active windows
    content->mut_at(win->get_layer()).second->mark_dirty();
    win->mark_dirty();

    content->mut_at(win->get_layer()).second = win;
}


AbstractWindow * AbstractPage::get_active_window()
{
    if (layer_Help_enabled)
        return content->mut_at(Layer::Help::grab()).second;

    if (layer_F_enabled)
        return content->mut_at(Layer::F::grab()).second;

    return content->mut_at(Layer::Bottom::grab()).second;
}


AbstractWindow * AbstractPage::get_active_window(Layer::Type layer)
{
    return content->mut_at(layer).second;
}


void AbstractPage::on_KEY(int key)
{
    if (!is_active())
        return;

    switch (key)
    {
        case KEY_F(1)    :
        case KEY_F(2)    :
        case KEY_F(3)    :
        case KEY_F(4)    :
        case KEY_F(5)    :
        case KEY_F(6)    :
        case KEY_F(7)    :
        case KEY_F(8)    :
        case KEY_F(9)    :
        case KEY_F(10)   :
        case KEY_F(11)   :
        case KEY_F(12)   : on_ANY_F();       return;
        case ','         : on_COMMA();       return;
        case SHIFT_LEFT  : on_SHIFT_LEFT();  return;
        case SHIFT_RIGHT : on_SHIFT_RIGHT(); return;
    }

    auto active_win = get_active_window();
    if (nullptr != active_win)
    {
        if (active_win->get_layer() == Layer::Help::grab())
        {
            switch (key)
            {
                case KEY_LEFT  : on_SHIFT_LEFT();  return;
                case KEY_RIGHT : on_SHIFT_RIGHT(); return;
            }
        }

        active_win->on_KEY(key);
    }
}


void AbstractPage::on_ANY_F()
{
    if (layer_Help_enabled)
        return;

    toggle_layer(Layer::F::grab(), layer_F_enabled);
}


void AbstractPage::on_COMMA()
{
    toggle_layer(Layer::Help::grab(), layer_Help_enabled);
}


void AbstractPage::toggle_layer(Layer::Type layer, bool & layer_enabled)
{
    if (layer_enabled)
    {
        // disable layer
        layer_enabled = false;
        for (auto w : content->mut_at(layer).first)
            w->disable(VisibilityAspect::WindowVisibility::grab());

        // mark other layers dirty
        for (auto l : Layer::variants())
            if (l != layer)
                for (auto w : content->mut_at(l).first)
                    w->mark_dirty();
    }
    else
    {
        // enable layer
        layer_enabled = true;
        for (auto w : content->mut_at(layer).first)
            w->enable(VisibilityAspect::WindowVisibility::grab());
    }
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
