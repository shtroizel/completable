#pragma once

#include "AbstractListWindow.h"


class InputWindow;
class CompletionWindow;
class SynonymWindow;
struct word_filter;


/**
 * AntonymWindow lists antonyms of the currently completed word
 */
class AntonymWindow : public AbstractListWindow
{
public:
    AntonymWindow(
        CompletionStack &,
        WordStack &,
        InputWindow &,
        CompletionWindow &,
        SynonymWindow &,
        word_filter &
    );

private:
    // resolved AbstractWindow dependencies
    std::string title() final;
    void resize_hook() final;

    // resolved AbstractListWindow dependencies
    int & display_start() final;
    void unfiltered_words(int, int const * *, int *) const final;
    bool apply_filter() const final { return true; }

    // AbstractListWindow options
    void on_post_RETURN() final;

    SynonymWindow & syn_win;
};
