#include "AntonymWindow.h"

#include <ncurses.h>

#include "AbstractTab.h"
#include "CompletionStack.h"
#include "InputWindow.h"
#include "CompletionWindow.h"
#include "SynonymWindow.h"
#include "matchmaker.h"



AntonymWindow::AntonymWindow(
    CompletionStack & cs,
    WordStack & ws,
    InputWindow & iw,
    CompletionWindow & cw,
    SynonymWindow & sw,
    word_filter & f
)
    : AbstractListWindow(cs, ws, iw, f)
    , syn_win(sw)
{
    cw.add_dirty_dependency(this);
}


std::string AntonymWindow::title()
{
    std::string t{"Antonyms ("};

    auto ant = get_words();
    t += std::to_string(ant.size());

    t += ")";

    return t;
}


void AntonymWindow::resize_hook()
{
    y = syn_win.get_y() + syn_win.get_height();
    x = syn_win.get_x();
    height = root_y - y - 5;
    width = syn_win.get_width();
}


int & AntonymWindow::display_start()
{
    return cs.top().ant_display_start;
}


void AntonymWindow::on_post_RETURN()
{
    AbstractTab::get_active_tab().as_AbstractTab()->set_active_window(&syn_win);
}


void AntonymWindow::unfiltered_words(int index, int const * * words, int * count) const
{
    matchmaker::antonyms(index, words, count);
}
