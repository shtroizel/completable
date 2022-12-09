#pragma once

#include "AbstractWindow.h"



/**
 * The MatchmakerHelpWindow provides help on window usage within the "matchmaker" tab
 */
class MatchmakerHelpWindow : public AbstractWindow
{
    using AbstractWindow::AbstractWindow;

    // resolved dependencies
    std::string title() final;
    void resize_hook() final;
    void draw_hook() final;
    Layer::Type layer() const final;
};
