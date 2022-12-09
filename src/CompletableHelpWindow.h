#pragma once

#include "AbstractWindow.h"



/**
 * The CompletableHelpWindow provides help on window usage within the "completable" tab
 */
class CompletableHelpWindow : public AbstractWindow
{
    using AbstractWindow::AbstractWindow;

    std::string title() final;
    void resize_hook() final;
    void draw_hook() final;
    Layer::Type layer() const final;
};
