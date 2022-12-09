#include "SettingsHelpWindow.h"

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
        content.push_back("            FEATURE   KEY(s)");
        content.push_back("   ------------------------------------------------------   ");
        content.push_back("        toggle help   ','");
        content.push_back("         switch tab   shift + arrow left/right, ");
        content.push_back("                      or arrow left/right when help shown");
        content.push_back("     change setting   Return");
        content.push_back("   change selection   arrow up/down");
        content.push_back("   enter shell mode   any of '$', '~', '`'");
        content.push_back("               quit   Esc, ctrl + c");
        content.push_back("");
        return content;
    }();



std::string SettingsHelpWindow::title()
{
    static std::string const t{"Help"};
    return t;
}


void SettingsHelpWindow::resize_hook()
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


void SettingsHelpWindow::draw_hook()
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


Layer::Type SettingsHelpWindow::layer() const
{
    return Layer::Help::grab();
}
