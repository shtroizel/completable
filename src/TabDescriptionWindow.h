#pragma once

#include "AbstractWindow.h"



/**
 * TabDescriptionWindow shows the active tab's name
 */
class TabDescriptionWindow : public AbstractWindow
{
    using AbstractWindow::AbstractWindow;

    // resolved AbstractWindow dependencies
    std::string title() final;
    void resize_hook() final;
    void draw_hook() final;
    Layer::Type layer() const final;

    // AbstractWindow options
    void post_resize_hook() final;
    bool borders_enabled() const final { return false; }
};
