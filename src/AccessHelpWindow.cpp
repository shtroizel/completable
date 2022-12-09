#include "AccessHelpWindow.h"

#include <ncurses.h>

#include "CompletionStack.h"
#include "Layer.h"



std::string AccessHelpWindow::title()
{
    static std::string const t{"press ',' for help"};
    return t;
}


void AccessHelpWindow::resize_hook()
{
    height = 3;
    width = root_x / 2;
    y = 0;
    x = root_x / 2 - width / 2;
}


Layer::Type AccessHelpWindow::layer() const
{
    return Layer::Bottom::grab();
}
