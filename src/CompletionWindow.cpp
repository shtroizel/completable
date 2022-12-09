#include "CompletionWindow.h"

#include <ncurses.h>

#include "CompletionStack.h"
#include "InputWindow.h"
#include "Settings.h"



std::string CompletionWindow::title()
{
    std::string t{"Completion ("};
    t += std::to_string(cs.top().standard_completion.size());
    t += ")";
    return t;
}


void CompletionWindow::resize_hook()
{
    y = 3;
    x = 0;

    if (EnablednessSetting::CompletionList::grab().as_enabledness() == Enabledness::Enabled::grab())
    {
        int combined_height = root_y - 5 - y;

        if (EnablednessSetting::Length_spc_Completion::grab().as_enabledness() == Enabledness::Enabled::grab()
            || EnablednessSetting::Ordinal_spc_Summation::grab().as_enabledness() == Enabledness::Enabled::grab())
            height = combined_height / 1.618;
        else
            height = combined_height;
    }
    else
    {
        height = 3;
    }

    width = root_x / 2;
}


int & CompletionWindow::display_start()
{
    return cs.top().display_start;
}


void CompletionWindow::unfiltered_words(int, int const * * words, int * count) const
{
    *words = cs.top().standard_completion.data();
    *count = (int) cs.top().standard_completion.size();
}
