#pragma once

#include "AbstractListWindow.h"


class InputWindow;
class CompletionWindow;
struct word_filter;


/**
 * OrdinalSummationWindow lists words sharing the same ordinal summation as the currently completed word
 */
class OrdinalSummationWindow : public AbstractListWindow
{
public:
    OrdinalSummationWindow(
        CompletionStack &,
        WordStack &,
        InputWindow &,
        word_filter &,
        CompletionWindow &
    );

private:
    // resolved AbstractWindow dependencies
    std::string title() final;
    void resize_hook() final;

    // resolved AbstractListWindow dependencies
    int & display_start() final;
    void unfiltered_words(int, int const * *, int *) const final;
    bool apply_filter() const final { return true; }

    CompletionWindow & completion_win;
};
