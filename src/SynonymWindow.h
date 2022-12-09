#pragma once

#include "AbstractListWindow.h"


class CompletionWindow;


/**
 * SynonymWindow lists synonyms of the currently completed word
 */
class SynonymWindow : public AbstractListWindow
{
public:
    SynonymWindow(CompletionStack &, WordStack &, InputWindow &, CompletionWindow &, word_filter &);

private:
    // resolved AbstractWindow dependencies
    std::string title() final;
    void resize_hook() final;

    // resolved AbstractListWindow dependencies
    int & display_start() final;
    void unfiltered_words(int, int const * *, int *) const final;
    bool apply_filter() const final { return true; }
};
