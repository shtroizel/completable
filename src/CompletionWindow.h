#pragma once

#include "AbstractListWindow.h"



/**
 * The CompletionWindow class provides the window for showing all words starting with the given input
 */
class CompletionWindow : public AbstractListWindow
{
    using AbstractListWindow::AbstractListWindow;

    // resolved AbstractWindow dependencies
    std::string title() final;
    void resize_hook() final;

    // resolved AbstractListWindow dependencies
    int & display_start() final;
    void unfiltered_words(int, int const * *, int *) const final;
    bool apply_filter() const final { return false; }
};
