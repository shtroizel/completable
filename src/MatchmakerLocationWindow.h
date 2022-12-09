#pragma once

#include "AbstractWindow.h"

#include <string>



class MatchmakerSelectionWindow;

/**
 * MatchmakerLocationWindow provides an area for users to type a file system path where dictionaries
 * should be searched.
 */
class MatchmakerLocationWindow : public AbstractWindow
{
public:
    explicit MatchmakerLocationWindow(MatchmakerSelectionWindow &);

private:
    // resolved dependencies
    std::string title() final;
    void resize_hook() final;
    void draw_hook() final;
    Layer::Type layer() const final;

    // options
    void on_TAB() final;
    void on_printable_ascii(int) final;
    void on_BACKSPACE() final;
    void on_RETURN() final;
    void on_KEY_UP() final;
    void on_KEY_DOWN() final;

    MatchmakerSelectionWindow & mm_sel_win;
    std::string search_prefix;
};
