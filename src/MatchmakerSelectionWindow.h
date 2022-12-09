#pragma once

#include "AbstractWindow.h"

#include <string>



/**
 * MatchmakerSelectionWindow provides the list of found dictionaries after searching the path provided
 * to MatchmakerLocationWindow. MatchmakerSelectionWindow also provides selection and application of
 * dictionaries
 */
class MatchmakerSelectionWindow : public AbstractWindow
{
public:
    void set_content(std::vector<std::string>);

    void load_currently_selected();
    void select_previous();
    void select_next();

private:
    // resolved dependencies
    std::string title() final;
    void resize_hook() final;
    void draw_hook() final;
    Layer::Type layer() const final;

    // options
    bool borders_enabled() const final { return false; } // draw our own instead

    std::vector<std::string> content;
    int selected{0};
};
