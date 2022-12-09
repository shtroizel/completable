#include "OrdinalSummationWindow.h"

#include <ncurses.h>

#include "AbstractTab.h"
#include "CompletionStack.h"
#include "InputWindow.h"
#include "CompletionWindow.h"
#include "matchmaker.h"



OrdinalSummationWindow::OrdinalSummationWindow(
    CompletionStack & cs,
    WordStack & ws,
    InputWindow & iw,
    word_filter & f,
    CompletionWindow & cw
)
    : AbstractListWindow(cs, ws, iw, f)
    , completion_win(cw)
{
    completion_win.add_dirty_dependency(this);
}


std::string OrdinalSummationWindow::title()
{
    std::string t{"Ordinal Summation: "};

    auto const & completion = cs.top().standard_completion;
    if (completion.size() > 0)
        t += std::to_string(matchmaker::ordinal_summation(completion[cs.top().display_start]));
    else
        t += "0";

    t += "  (";
    auto os = get_words();
    t += std::to_string(os.size());
    t += ")";

    return t;
}


void OrdinalSummationWindow::resize_hook()
{
	y = completion_win.get_y() + completion_win.get_height();
    x = 0;
    height = root_y - y - 5;
    width = completion_win.get_width();
}


int & OrdinalSummationWindow::display_start()
{
    return cs.top().ord_sum_display_start;
}


void OrdinalSummationWindow::unfiltered_words(int index, int const * * words, int * count) const
{
    matchmaker::from_ordinal_summation(matchmaker::ordinal_summation(index), words, count);
}
