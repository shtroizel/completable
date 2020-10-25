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



#include "AbstractTab.h"

#include <ncurses.h>

#include "AbstractWindow.h"
#include "EnablednessSetting.h"
#include "Layer.h"
#include "VisibilityAspect.h"
#include "key_codes.h"



AbstractTab::AbstractTab()
    : layers{
        std::make_shared<
            matchable::MatchBox<
                Layer::Type,
                std::pair<std::vector<AbstractWindow *>, AbstractWindow *>
            >
        >()
      }
{
    for (auto l : Layer::variants())
        layers->mut_at(l).second = nullptr;

    EnablednessSetting::Borders::grab().add_enabledness_observer([&](){ draw(true); });

    Tab::nil.set_AbstractTab(nullptr);
}


AbstractTab::~AbstractTab()
{
    layers.reset();
    active_tab() = nullptr;
    left_neighbor = Tab::nil;
    right_neighbor = Tab::nil;
}


void AbstractTab::add_window(AbstractWindow * win)
{
    if (nullptr == win)
        return;

    layers->mut_at(win->get_layer()).first.push_back(win);
    win->add_tab(as_handle());

    // guarantee active window by setting first window active
    if (layers->at(win->get_layer()).first.size() == 1)
        layers->mut_at(win->get_layer()).second = win;
}


void AbstractTab::resize()
{
    if (is_active())
        for (auto l : Layer::variants())
            for (auto w : layers->mut_at(l).first)
                w->resize();
}


void AbstractTab::draw(bool clear_first)
{
    if (is_active())
    {
        for (auto w : layers->mut_at(Layer::Bottom::grab()).first)
            w->draw(clear_first);

        if (layer_F_enabled)
            for (auto w : layers->mut_at(Layer::F::grab()).first)
                w->draw(clear_first);

        if (layer_Help_enabled)
            for (auto w : layers->mut_at(Layer::Help::grab()).first)
                w->draw(clear_first);
    }
}



void AbstractTab::set_active_tab(Tab::Type tab)
{
    if (tab.is_nil())
        return;

    if (nullptr != active_tab())
        for (auto l : Layer::variants())
            for (auto w : active_tab()->layers->mut_at(l).first)
                w->disable(VisibilityAspect::TabVisibility::grab());

    AbstractWindow const * tab_act_win = tab.as_AbstractTab()->get_active_window();

    // synce WindowVisibility aspect for Help layer, specifically:
    //
    // if help enabled for old tab then enable help for new tab if not already
    // and...
    // if help disabled for old tab then disable help for new tab if not already
    if (nullptr != active_tab() && nullptr != tab_act_win)
    {
        // if help enabled for old/current active_tab()
        if (nullptr != active_tab()->get_active_window() &&
                active_tab()->get_active_window()->get_layer() == Layer::Help::grab())
        {

            // then enable if help disabled for new active_tab()
            if (tab_act_win->get_layer() != Layer::Help::grab())
                tab.as_AbstractTab()->on_COMMA();
        }
        else // otherwise if help disabled for old/current active_tab()
        {
            // then disable if help enabled for new active_tab()
            if (tab_act_win->get_layer() == Layer::Help::grab())
                tab.as_AbstractTab()->on_COMMA();
        }
    }

    active_tab() = tab.as_AbstractTab();

    // enable TabVisibility aspect for each window within each layer
    if (nullptr != active_tab())
        for (auto l : Layer::variants())
            for (auto w : active_tab()->layers->mut_at(l).first)
                w->enable(VisibilityAspect::TabVisibility::grab());
}


Tab::Type AbstractTab::get_active_tab()
{
    if (nullptr == active_tab())
        return Tab::nil;

    return active_tab()->as_handle();
}


void AbstractTab::set_active_window(AbstractWindow * win)
{
    if (nullptr == win)
        return;

    // verify given window exists in the given layer
    {
        bool found = false;
        for (auto w : layers->at(win->get_layer()).first)
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
    layers->mut_at(win->get_layer()).second->mark_dirty();
    win->mark_dirty();

    layers->mut_at(win->get_layer()).second = win;
}


AbstractWindow * AbstractTab::get_active_window()
{
    if (layer_Help_enabled)
        return layers->mut_at(Layer::Help::grab()).second;

    if (layer_F_enabled)
        return layers->mut_at(Layer::F::grab()).second;

    return layers->mut_at(Layer::Bottom::grab()).second;
}


AbstractWindow * AbstractTab::get_active_window(Layer::Type layer)
{
    return layers->mut_at(layer).second;
}


void AbstractTab::on_KEY(int key)
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


void AbstractTab::on_ANY_F()
{
    if (layer_Help_enabled)
        return;

    toggle_layer(Layer::F::grab(), layer_F_enabled);
}


void AbstractTab::on_COMMA()
{
    toggle_layer(Layer::Help::grab(), layer_Help_enabled);
}


void AbstractTab::toggle_layer(Layer::Type layer, bool & layer_enabled)
{
    if (layer_enabled)
    {
        // disable layer
        layer_enabled = false;
        for (auto w : layers->mut_at(layer).first)
            w->disable(VisibilityAspect::WindowVisibility::grab());

        // mark other layers dirty
        for (auto l : Layer::variants())
            if (l != layer)
                for (auto w : layers->mut_at(l).first)
                    w->mark_dirty();
    }
    else
    {
        // enable layer
        layer_enabled = true;
        for (auto w : layers->mut_at(layer).first)
            w->enable(VisibilityAspect::WindowVisibility::grab());
    }
}


void AbstractTab::on_SHIFT_LEFT()
{
    if (!left_neighbor.is_nil())
        AbstractTab::set_active_tab(left_neighbor);
}


void AbstractTab::on_SHIFT_RIGHT()
{
    if (!right_neighbor.is_nil())
        AbstractTab::set_active_tab(right_neighbor);
}
