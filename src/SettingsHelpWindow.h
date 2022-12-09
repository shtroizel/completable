#pragma once

#include "AbstractWindow.h"



/**
 * The SettingsHelpWindow provides help on window usage within the "settings" tab
 */
class SettingsHelpWindow : public AbstractWindow
{
    using AbstractWindow::AbstractWindow;

    // resolved dependencies
    std::string title() final;
    void resize_hook() final;
    void draw_hook() final;
    Layer::Type layer() const final;
};
