#include "TabDescriptionWindow.h"

#include <ncurses.h>

#include "AbstractTab.h"
#include "Layer.h"



std::string TabDescriptionWindow::title()
{
    static std::string const t;
    return t;
}


void TabDescriptionWindow::resize_hook()
{
    height = 3;
    width = 17;
    y = 0;
    x = 1;
}


void TabDescriptionWindow::draw_hook()
{
    auto tab = AbstractTab::get_active_tab();
    if (tab.is_nil())
        return;

    for (int i = 0; i < (int) tab.as_string().length() && i < width; ++i)
        mvwaddch(w, 1, i, tab.as_string()[i]);
}


void TabDescriptionWindow::post_resize_hook()
{
    // TabDescriptionWindow is a window that is on every tab.
    // As such, it has been recruited to be the window used for the keypad.
    // All key events regardless of what tab or window is active come through
    // TabDescriptionWindow's low level ncurses WINDOW.
    keypad(w, true);
}


Layer::Type TabDescriptionWindow::layer() const
{
    return Layer::Bottom::grab();
}
