#pragma once

#include <array>
#include <memory>
#include <vector>

#include <matchable/matchable.h>
#include <matchable/matchable_fwd.h>



MATCHABLE_FWD(Layer)

class AbstractTab;
class AbstractWindow;


// handle for AbstractTab types
// derivers need to use SPREAD_MATCHABLE() to add their types
// the "AbstractTab" property allows retrieval of the "real" tab
PROPERTYx1_MATCHABLE(AbstractTab *, AbstractTab, Tab)

/**
 * AbstractTab serves as the base for all Tabs, which provide a space for windows that should be
 * seen together.
 *
 * Windows are known by Tabs but not owned by them (*TabAgent classes take care of ownership).
 *
 * Tabs manage windows within layers
 * Each window specifies which layer it belongs to with AbstractWindow::layer()
 *   - A window's defined layer determines its stacking order, with Layer::Help above all others
 *   - Key events are caught here first to handle Layer enabledness before being forwarded to the active
 *     window
 *   - Each layer has its own active window, with the top most receiving any unhandled key events
 */
class AbstractTab
{
public:
    AbstractTab(AbstractTab const &) = delete;
    AbstractTab & operator=(AbstractTab const &) = delete;

    AbstractTab();
    virtual ~AbstractTab();

    /**
     * @returns A handle to the tab with access back to the tab via matchable property
     */
    Tab::Type as_handle() const { return as_matchable(); }

    /**
     * Adds a window to the tab. Ownership is preserved.
     * Also, the tab will add its handle to the window for back-referencing
     *
     * @param[in] win Window to be added
     */
    void add_window(AbstractWindow * win);

    /**
     * recalculate size and position for all windows managed by the tab
     */
    void resize();

    /**
     * redraw all windows managed by the tab
     * @see AbstractWindow::draw()
     */
    void draw(bool clear_first);

    /**
     * Set the tab's left neighbor.
     * When switching tabs with shift + left arrow, this tab is to be the next tab shown
     * @param[in] neighbor handle to the tab's left neighbor
     */
    void set_left_neighbor(Tab::Type neighbor) { left_neighbor = neighbor; }

    /**
     * Set the tab's right neighbor.
     * When switching tabs with shift + right arrow, this tab is to be the next tab shown
     * @param[in] neighbor handle to the tab's right neighbor
     */
    void set_right_neighbor(Tab::Type neighbor) { right_neighbor = neighbor; }

    /**
     * Set which tab should be visible (activated). One tab is visible at a time.
     * @param[in] handle to a tab to be activated
     */
    static void set_active_tab(Tab::Type tab);

    /**
     * @returns A handle to the currently active tab
     */
    static Tab::Type get_active_tab();

    /**
     * @returns true if the tab is currently active or false otherwise
     */
    bool is_active() { return get_active_tab() == as_handle(); }

    /**
     * Sets the active window for the layer that the given window belongs to. Note that if the window's
     * layer is disabled or lies underneath another enabled layer then the window returned by
     * get_active_window() may remain unchanged
     *
     * @param[in] win new active window for the layer specified by win->get_layer()
     */
    void set_active_window(AbstractWindow * win);

    /**
     * @returns the active window for the top-most enabled layer
     */
    AbstractWindow * get_active_window();

    /**
     * @param[in] layer The layer for which to retrieve an active window
     * @returns the active window for the given layer
     */
    AbstractWindow * get_active_window(Layer::Type layer);

    /**
     * Event handler for keyboard
     * Keys used for tab switching or layer enabledness are caught while unhandled keys are forwarded
     *     to the active window of the top-most enabled layer
     *
     * @param[in] key keyboard key to be handled
     */
    void on_KEY(int key);

    /**
     * Indicator positions start at 0 and go from right to left
     *
     * @returns The tab's indicator position (example: third indicator from the right has position 2)
     */
    int get_indicator_position() const { return indicator_position(); }


private:
    // dependencies
    virtual int indicator_position() const = 0;
    virtual Tab::Type as_matchable() const = 0;

    void on_ANY_F();
    void on_COMMA();
    void toggle_layer(Layer::Type, bool &);
    void on_SHIFT_LEFT();
    void on_SHIFT_RIGHT();


private:
    Tab::Type left_neighbor;
    Tab::Type right_neighbor;

    std::shared_ptr<
        matchable::MatchBox<
            Layer::Type,
            // first for all windows in layer, second for layer's active window
            std::pair<std::vector<AbstractWindow *>, AbstractWindow *>
        >
    > layers;
    bool layer_F_enabled{false};
    bool layer_Help_enabled{false};

    // static variable for the active tab (look like a function but its a variable)
    static AbstractTab * & active_tab() { static AbstractTab * tab; return tab; }
};
