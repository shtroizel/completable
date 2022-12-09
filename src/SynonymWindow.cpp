#include "SynonymWindow.h"

#include <ncurses.h>

#include "CompletionWindow.h"
#include "CompletionStack.h"
#include "Settings.h"
#include "InputWindow.h"
#include "LengthCompletionWindow.h"
#include "matchmaker.h"
#include "word_filter.h"



SynonymWindow::SynonymWindow(
    CompletionStack & cs,
    WordStack & ws,
    InputWindow & iw,
    CompletionWindow & cw,
    word_filter & wf
)
    : AbstractListWindow(cs, ws, iw, wf)
{
    cw.add_dirty_dependency(this);
}


std::string SynonymWindow::title()
{
    std::string t{"Synonyms ("};

    auto syn = get_words();
    t += std::to_string(syn.size());

    t += ")";

    return t;
}


void SynonymWindow::resize_hook()
{
    y = 3;
    x = root_x / 2;

    int combined_height = root_y - 5 - y;
    if (EnablednessSetting::Antonyms::grab().as_enabledness() == Enabledness::Enabled::grab())
        height = combined_height / 1.618;
    else
        height = combined_height;

    width = x + root_x % 2;
}


int & SynonymWindow::display_start()
{
    return cs.top().syn_display_start;
}


void SynonymWindow::unfiltered_words(int index, int const * * words, int * count) const
{
    matchmaker::synonyms(index, words, count);
}
