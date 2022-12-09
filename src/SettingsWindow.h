#pragma once

#include "AbstractWindow.h"



/**
 * SettingsWindow provides a way to view and edit the application's settings
 */
class SettingsWindow : public AbstractWindow
{
    using AbstractWindow::AbstractWindow;

    // resolved dependencies
    std::string title() final;
    void resize_hook() final;
    void draw_hook() final;
    Layer::Type layer() const final;

    // options
    void on_RETURN() final;
    void on_KEY_UP() final;
    void on_KEY_DOWN() final;

    int selection{0};
};
