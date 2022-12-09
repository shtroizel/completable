#include "CompletableHelpWindow.h"

#include <string>
#include <vector>

#include <ncurses.h>

#include "InputWindow.h"
#include "Layer.h"
#include "VisibilityAspect.h"



static std::vector<std::string> const content =
    []()
    {
        std::vector<std::string> content;
        content.push_back("");
        content.push_back("                FEATURE   KEY(s)");
        content.push_back("   ------------------------------------------------------------   ");
        content.push_back("            toggle help   ','");
        content.push_back("             switch tab   shift + arrow left/right,");
        content.push_back("                          or arrow left/right when help shown");
        content.push_back("           update input   letters");
        content.push_back("   complete unambiguous   tab");
        content.push_back("    change window focus   arrow left/right");
        content.push_back("              scrolling   arrow up/down, page up/down, home/end");
        content.push_back("       push input stack   Return");
        content.push_back("        pop input stack   Del");
        content.push_back("   toggle filter window   any F key (F1..F12)");
        content.push_back("       enter shell mode   any of '$', '~', '`'");
        content.push_back("                   quit   Esc, ctrl + c");
        content.push_back("");
        return content;
    }();



std::string CompletableHelpWindow::title()
{
    static std::string const t{"Help"};
    return t;
}


void CompletableHelpWindow::resize_hook()
{
    height = (int) content.size() + 2; // 2 for top/bottom borders

    static int const content_width =
        []()
        {
            int content_width = 0;
            for (auto line : content)
                if ((int) line.length() > content_width)
                    content_width = line.length();
            return content_width;
        }();
    width = content_width + 2; // 2 for left/right borders

    // center window
    y = (root_y - height) / 2;
    x = (root_x - width) / 2;
}


void CompletableHelpWindow::draw_hook()
{
    int i = 0;
    for (; i < (int) content.size() && i < height; ++i)
    {
        std::string const & line = content[i];

        int j = 0;
        for (; j < (int) line.size() && j < width - 2; ++j)
            mvwaddch(w, i + 1, j + 1, line[j]);

        // blank out rest of line
        for (; j < width - 2; ++j)
            mvwaddch(w, i + 1, j + 1, ' ');
    }

    // blank out remaining lines
    for (; i < height - 2; ++i)
        for (int j = 0; j < width - 2; ++j)
            mvwaddch(w, i + 1, j + 1, ' ');
}


Layer::Type CompletableHelpWindow::layer() const
{
    return Layer::Help::grab();
}
