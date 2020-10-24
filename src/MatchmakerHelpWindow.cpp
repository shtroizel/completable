/*
Copyright (c) 2020, Eric Hyer
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/



#include "MatchmakerHelpWindow.h"

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
        content.push_back("                  FEATURE   KEY(s)");
        content.push_back("   ------------------------------------------------------------   ");
        content.push_back("              toggle help   ','");
        content.push_back("               switch tab   shift + arrow left/right, ");
        content.push_back("                            or arrow left/right when help shown");
        content.push_back("   update search location   letters, symbols");
        content.push_back("    search given location   Tab");
        content.push_back("         change selection   arrow up/down");
        content.push_back("        select dictionary   Return");
        content.push_back("         enter shell mode   any of '$', '~', '`'");
        content.push_back("                     quit   Esc, ctrl + c");
        content.push_back("");
        return content;
    }();



std::string MatchmakerHelpWindow::title()
{
    static std::string const t{"Help"};
    return t;
}


void MatchmakerHelpWindow::resize_hook()
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


void MatchmakerHelpWindow::draw_hook()
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


Layer::Type MatchmakerHelpWindow::layer() const
{
    return Layer::Help::grab();
}
