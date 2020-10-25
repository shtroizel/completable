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


#include <functional>
#include <memory>
#include <stack>
#include <string>
#include <vector>

#include <matchable/matchable_fwd.h>

#include "word_stack_element.h"


// ncurses window
typedef struct _win_st WINDOW;

class AbstractTab;
class CompletionStack;
using WordStack = std::stack<word_stack_element>;
MATCHABLE_FWD(Layer);
MATCHABLE_FWD(Tab);
MATCHABLE_FWD(VisibilityAspect);


/**
 * Only AbstractTab may call the AbstractWindow::add_tab() function
 */
class AccessKey_AbstractWindow_add_tab { friend AbstractTab; AccessKey_AbstractWindow_add_tab() {} };


/**
 * AbstractWindow serves as the base for all windows, wrapping the low-level ncurses WINDOW type.
 */
class AbstractWindow
{
public:
    AbstractWindow(AbstractWindow const &) = delete;
    AbstractWindow & operator=(AbstractWindow const &) = delete;

    AbstractWindow();
    virtual ~AbstractWindow();

    /**
     * clear and refresh the window
     */
    void clear();

    /**
     * delete and recreate the underlying ncurses WINDOW using the size and location specified by
     * the deriver's resize_hook()
     *
     * derivers may optionally implement post_resize_hook() if they need to react on resize after the
     * new underlying WINDOW has been created.
     */
    void resize();

    /**
     * Conditionally redraws the window if it is enabled and has been marked dirty
     * @see is_enabled()
     * @see mark_dirty()
     *
     * @param clear_first When true the window is first cleared before drawing. Furthermore, when true
     *     enabled windows are always redrawn regardless of whether they have been marked dirty or not
     */
    void draw(bool clear_first);

    /**
     * Windows are assigned to "layers" that define their stacking order. Each window belongs to exactly
     * one layer. Derivers define this by implementing the layer() dependency.
     *
     * @returns The layer in which the window resides
     */
    Layer::Type get_layer() const;

    /**
     * @returns The window's height
     */
    int get_height() const { return height; }

    /**
     * @returns The window's width
     */
    int get_width() const { return width; }

    /**
     * @returns The window's y position within the ncurses "stdscr"
     */
    int get_y() const { return y; }

    /**
     * @returns The window's x position within the ncurses "stdscr"
     */
    int get_x() const { return x; }

    /**
     * @returns The underlying ncurses WINDOW
     */
    WINDOW * get_WINDOW() const { return w; }

    /**
     * Titles are provided by derivers by implementing title()
     *
     * @returns The windows title
     */
    std::string get_title() { return title(); }

    /**
     * Defines window switching behavior when switching to the left.
     * Neighbors need to be set per tab since windows can exist in more than one tab.
     *
     * @param[in] tab A tab where this window has been added (@see AbstractTab::add_window())
     * @param[in] neighbor The window to switch to when switching to the left
     */
    void set_left_neighbor(Tab::Type tab, AbstractWindow * neighbor);

    /**
     * Defines window switching behavior when switching to the right.
     * Neighbors need to be set per tab since windows can exist in more than one tab.
     *
     * @param[in] tab A tab where this window has been added (@see AbstractTab::add_window())
     * @param[in] neighbor The window to switch to when switching to the right
     */
    void set_right_neighbor(Tab::Type tab, AbstractWindow * neighbor);

    /**
     * @param[in] tab A tab where this window has been added (@see AbstractTab::add_window())
     * @returns The windows left neighbor
     * @see set_left_neighbor()
     */
    AbstractWindow * get_left_neighbor(Tab::Type tab);

    /**
     * @param[in] tab A tab where this window has been added (@see AbstractTab::add_window())
     * @returns The windows right neighbor
     * @see set_right_neighbor()
     */
    AbstractWindow * get_right_neighbor(Tab::Type tab);

    /**
     * @returns true if this window is the active window of the top-most enabled layer
     *     of the currently active tab, or false otherwise
     * @see AbstractTab::get_active_window()
     */
    bool is_active();

    /**
     * @returns true if this window has been added to the currently active tab, or false otherwise
     */
    bool belongs_to_active_tab();

    /**
     * Only callable by AbstractTab, this function will add a back-reference handle to a tab when
     * the window is added with AbstractTab::add_window()
     *
     * @param key Only AbstractTab has the key (@see AccessKey_AbstractWindow_add_tab)
     * @param[in] tab The handle back-referencing the tab to which this window was added
     */
    void add_tab(AccessKey_AbstractWindow_add_tab key, Tab::Type tab);

    /**
     * Event handler for keyboard keys
     *
     * @param[in] key keyboard key to be handled
     */
    void on_KEY(int key);

    /**
     * Allows draw() to always be called within an event loop while maintaining efficiency.
     * Only when the window is "dirty" will draw be performed.
     * Use mark_dirty() to signify that the window needs to be redrawn
     */
    void mark_dirty();

    /**
     * "Dirty dependencies" are windows that inherit this window's dirtiness.
     * In other words, redraw these windows too when this window is dirty
     *
     * @param[in] win A window that should be redrawn when this window becomes dirty
     */
    void add_dirty_dependency(AbstractWindow * win);

    /**
     * Set the window's enabledness for some given VisibilityAspect.
     * A window may be enabled or disabled for various reasons.
     * Maybe the tab has been switched.
     * Maybe the window's tab is active but the window's layer has been disabled.
     * Such reasons are referred to as "aspects", and AbstractWindow maintains enabledness for each one
     *
     * @param[in] enabledness The enabledness state
     * @param[in] aspect The aspect to which the given enabledness state pertains
     */
    void set_enabled(bool enabledness, VisibilityAspect::Type aspect);

    /**
     * @returns true when all visibility aspects are enabled, or false if any aspect is disabled
     */
    bool is_enabled() const;

    /**
     * @param[in] aspect The aspect for which enabledness is queried
     * @returns The window's enabledness for the given visibility aspect
     */
    bool is_enabled(VisibilityAspect::Type aspect) const;

    /**
     * Enable the window for the given visibility aspect
     * @param[in] aspect The aspect for which this window is to be enabled
     */
    void enable(VisibilityAspect::Type aspect);

    /**
     * Disable the window for the given visibility aspect
     * @param[in] aspect The aspect for which this window is to be disabled
     */
    void disable(VisibilityAspect::Type aspect);

    /**
     * Activate the window's left neighbor for the given tab.
     * The tab must be specified since the window could exist in more than one tab and thus have
     * different neighbors in each tab
     *
     * @param[in] tab The tab in which the left neighbor is to be activated
     */
    void activate_left(Tab::Type tab);

    /**
     * Activate the window's right neighbor for the given tab.
     * The tab must be specified since the window could exist in more than one tab and thus have
     * different neighbors in each tab
     *
     * @param[in] tab The tab in which the right neighbor is to be activated
     */
    void activate_right(Tab::Type tab);


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

    // common implementation for activate_left() and activate_right()
    void activate_neighbor(Tab::Type, std::function<AbstractWindow * (AbstractWindow *, Tab::Type)>);


protected:
    WINDOW * w{nullptr};
    int root_y{0};
    int root_x{0};
    int height{0};
    int width{0};
    int y{0};
    int x{0};
    std::shared_ptr<Tab::Flags> tabs;

private:
    std::shared_ptr<VisibilityAspect::Flags> disabled;
    bool dirty{false};
    std::vector<AbstractWindow *> dirty_dependencies;
    std::shared_ptr<matchable::MatchBox<Tab::Type, AbstractWindow *>> left_neighbor;
    std::shared_ptr<matchable::MatchBox<Tab::Type, AbstractWindow *>> right_neighbor;
};
