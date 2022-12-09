#pragma once

#include "AbstractWindow.h"



/**
 * The AccessHelpWindow shows how to access help for tabs without InputWindow
 */
class AccessHelpWindow : public AbstractWindow
{
    using AbstractWindow::AbstractWindow;

    // resolved dependencies
    std::string title() final;
    void resize_hook() final;
    void draw_hook() final {}
    Layer::Type layer() const final;

    // options
    bool borders_enabled() const final { return false; }
};
