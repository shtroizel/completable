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



#include "FilterWindow.h"

#include <iostream>
#include <thread>

#include <ncurses.h>

#include "CompletionStack.h"
#include "InputWindow.h"
#include "Layer.h"
#include "word_filter.h"




FilterWindow::FilterWindow(CompletionStack & cs, WordStack & ws, InputWindow & iw, word_filter & f)
    : AbstractCompletionDataWindow(cs, ws)
    , input_win(iw)
    , wf(f)
{
}


FilterWindow::~FilterWindow()
{
}


std::string FilterWindow::title()
{
    static std::string const t{"Filter"};
    return t;
}


void FilterWindow::resize_hook()
{
    height = (int) (root_y / 1.618 + 0.5);
    width = (int) (root_x / 1.618 + 0.5);
    y = (root_y - height) / 2;
    x = (root_x - width) / 2;
}


void FilterWindow::post_resize_hook()
{
    keypad(w, true);
}


void FilterWindow::pre_disable_hook()
{
    static std::array<std::vector<std::string>, 20> const busy_content =
        []()
        {
            std::array<std::vector<std::string>, 20> content;

            std::vector<std::string> c0;
            c0.push_back("");
            c0.push_back("              |        WWG1WGA        |WWG1WGA|       ");
            c0.push_back("             AW       W      AW              A        ");
            c0.push_back("            G W       W     G W             G         ");
            c0.push_back("           W  G       G    W  G            W          ");
            c0.push_back("          1   1       1   1   1        WWG1WGA        ");
            c0.push_back("         G    W       W  G    W          G            ");
            c0.push_back("        W     G       G W     G         W             ");
            c0.push_back("       W      A       AW      A        W              ");
            c0.push_back("              |        WWG1WGA        |               ");
            c0.push_back("");
            content[0] = c0;

            std::vector<std::string> c1;
            c1.push_back("");
            c1.push_back("              /        WWG1WGA        /WWG1WGA/       ");
            c1.push_back("             AW       W      AW              A        ");
            c1.push_back("            G W       W     G W             G         ");
            c1.push_back("           W  G       G    W  G            W          ");
            c1.push_back("          1   1       1   1   1        WWG1WGA        ");
            c1.push_back("         G    W       W  G    W          G            ");
            c1.push_back("        W     G       G W     G         W             ");
            c1.push_back("       W      A       AW      A        W              ");
            c1.push_back("              /        WWG1WGA        /               ");
            c1.push_back("");
            content[1] = c1;

            std::vector<std::string> c2;
            c2.push_back("");
            c2.push_back("              -        WWG1WGA        -WWG1WGA-       ");
            c2.push_back("             AW       W      AW              A        ");
            c2.push_back("            G W       W     G W             G         ");
            c2.push_back("           W  G       G    W  G            W          ");
            c2.push_back("          1   1       1   1   1        WWG1WGA        ");
            c2.push_back("         G    W       W  G    W          G            ");
            c2.push_back("        W     G       G W     G         W             ");
            c2.push_back("       W      A       AW      A        W              ");
            c2.push_back("              -        WWG1WGA        -               ");
            c2.push_back("");
            content[2] = c2;

            std::vector<std::string> c3;
            c3.push_back("");
            c3.push_back("              \\        WWG1WGA        \\WWG1WGA\\       ");
            c3.push_back("             AW       W      AW              A        ");
            c3.push_back("            G W       W     G W             G         ");
            c3.push_back("           W  G       G    W  G            W          ");
            c3.push_back("          1   1       1   1   1        WWG1WGA        ");
            c3.push_back("         G    W       W  G    W          G            ");
            c3.push_back("        W     G       G W     G         W             ");
            c3.push_back("       W      A       AW      A        W              ");
            c3.push_back("              \\        WWG1WGA        \\               ");
            c3.push_back("");
            content[3] = c3;

            std::vector<std::string> c4;
            c4.push_back("");
            c4.push_back("              |        WWG1WGA        |WWG1WGA|       ");
            c4.push_back("             AW       W      AW              A        ");
            c4.push_back("            G W       W     G W             G         ");
            c4.push_back("           W  G       G    W  G            W          ");
            c4.push_back("          1   1       1   1   1        WWG1WGA        ");
            c4.push_back("         G    W       W  G    W          G            ");
            c4.push_back("        W     G       G W     G         W             ");
            c4.push_back("       W      A       AW      A        W              ");
            c4.push_back("              |        WWG1WGA        |               ");
            c4.push_back("");
            content[4] = c4;

            std::vector<std::string> c5;
            c5.push_back("");
            c5.push_back("              /        WG5WGA/        /WG5WGA//       ");
            c5.push_back("             /W       W      /W              /        ");
            c5.push_back("            A G       G     A G             A         ");
            c5.push_back("           G  5       5    G  5            G          ");
            c5.push_back("          W   W       W   W   W        WG5WGA/        ");
            c5.push_back("         5    G       G  5    G          5            ");
            c5.push_back("        G     A       A G     A         G             ");
            c5.push_back("       W      /       /W      /        W              ");
            c5.push_back("              /        WG5WGA/        /               ");
            c5.push_back("");
            content[5] = c5;

            std::vector<std::string> c6;
            c6.push_back("");
            c6.push_back("              -        G1WGA--        -G1WGA---       ");
            c6.push_back("             -G       G      -G              -        ");
            c6.push_back("            - 1       1     - 1             -         ");
            c6.push_back("           A  W       W    A  W            A          ");
            c6.push_back("          G   G       G   G   G        G1WGA--        ");
            c6.push_back("         W    A       A  W    A          W            ");
            c6.push_back("        1     -       - 1     -         1             ");
            c6.push_back("       G      -       -G      -        G              ");
            c6.push_back("              -        G1WGA--        -               ");
            c6.push_back("");
            content[6] = c6;

            std::vector<std::string> c7;
            c7.push_back("");
            c7.push_back("              \\        1WGA\\\\\\        \\1WGA\\\\\\\\       ");
            c7.push_back("             \\1       1      \\1              \\        ");
            c7.push_back("            \\ W       W     \\ W             \\         ");
            c7.push_back("           \\  G       G    \\  G            \\          ");
            c7.push_back("          A   A       A   A   A        1WGA\\\\\\        ");
            c7.push_back("         G    \\       \\  G    \\          G            ");
            c7.push_back("        W     \\       \\ W     \\         W             ");
            c7.push_back("       1      \\       \\1      \\        1              ");
            c7.push_back("              \\        1WGA\\\\\\        \\               ");
            c7.push_back("");
            content[7] = c7;

            std::vector<std::string> c8;
            c8.push_back("");
            c8.push_back("              |        WGA||||        |WGA|||||       ");
            c8.push_back("             |W       W      |W              |        ");
            c8.push_back("            | G       G     | G             |         ");
            c8.push_back("           |  A       A    |  A            |          ");
            c8.push_back("          |   |       |   |   |        WGA||||        ");
            c8.push_back("         A    |       |  A    |          A            ");
            c8.push_back("        G     |       | G     |         G             ");
            c8.push_back("       W      |       |W      |        W              ");
            c8.push_back("              |        WGA||||        |               ");
            c8.push_back("");
            content[8] = c8;

            std::vector<std::string> c9;
            c9.push_back("");
            c9.push_back("              /        GA/////        /GA//////       ");
            c9.push_back("             //       /      //              /        ");
            c9.push_back("            / /       /     / /             /         ");
            c9.push_back("           /  /       /    /  /            /          ");
            c9.push_back("          /   /       /   /   /        GA/////        ");
            c9.push_back("         /    /       /  /    /          /            ");
            c9.push_back("        A     G       G A     G         A             ");
            c9.push_back("       G      A       AG      A        G              ");
            c9.push_back("              /        GA/////        /               ");
            c9.push_back("");
            content[9] = c9;

            std::vector<std::string> c10;
            c10.push_back("");
            c10.push_back("              -        A------        -A-------       ");
            c10.push_back("             --       -      --              -        ");
            c10.push_back("            - -       -     - -             -         ");
            c10.push_back("           -  -       -    -  -            -          ");
            c10.push_back("          -   -       -   -   -        A------        ");
            c10.push_back("         -    -       -  -    -          -            ");
            c10.push_back("        -     -       - -     -         -             ");
            c10.push_back("       A      A       AA      A        A              ");
            c10.push_back("              -        A------        -               ");
            c10.push_back("");
            content[10] = c10;

            std::vector<std::string> c11;
            c11.push_back("");
            c11.push_back("              \\        \\\\\\\\\\\\\\        \\\\\\\\\\\\\\\\\\       ");
            c11.push_back("             \\\\       \\      \\\\              \\        ");
            c11.push_back("            \\ \\       \\     \\ \\             \\         ");
            c11.push_back("           \\  \\       \\    \\  \\            \\          ");
            c11.push_back("          \\   \\       \\   \\   \\        \\\\\\\\\\\\\\        ");
            c11.push_back("         \\    \\       \\  \\    \\          \\            ");
            c11.push_back("        \\     \\       \\ \\     \\         \\             ");
            c11.push_back("       \\      \\       \\\\      \\        \\              ");
            c11.push_back("              \\        \\\\\\\\\\\\\\        \\               ");
            c11.push_back("");
            content[11] = c11;

            std::vector<std::string> c12;
            c12.push_back("");
            c12.push_back("              |        |||||||        |||||||||       ");
            c12.push_back("             ||       |      ||              |        ");
            c12.push_back("            | |       |     | |             |         ");
            c12.push_back("           |  |       |    |  |            |          ");
            c12.push_back("          |   |       |   |   |        |||||||        ");
            c12.push_back("         |    |       |  |    |          |            ");
            c12.push_back("        |     |       | |     |         |             ");
            c12.push_back("       |      |       ||      |        |              ");
            c12.push_back("              |        |||||||        |               ");
            c12.push_back("");
            content[12] = c12;

            std::vector<std::string> c13;
            c13.push_back("");
            c13.push_back("              /        ///////        /////////       ");
            c13.push_back("             //       /      //              /        ");
            c13.push_back("            / /       /     / /             /         ");
            c13.push_back("           /  /       /    /  /            /          ");
            c13.push_back("          /   /       /   /   /        ///////        ");
            c13.push_back("         /    /       /  /    /          /            ");
            c13.push_back("        /     /       / /     /         /             ");
            c13.push_back("       /      /       //      /        /              ");
            c13.push_back("              /        ///////        /               ");
            c13.push_back("");
            content[13] = c13;

            std::vector<std::string> c14;
            c14.push_back("");
            c14.push_back("              -        ------W        -------W-       ");
            c14.push_back("             W-       -      W-              W        ");
            c14.push_back("            - -       -     - -             -         ");
            c14.push_back("           -  -       -    -  -            -          ");
            c14.push_back("          -   -       -   -   -        ------W        ");
            c14.push_back("         -    -       -  -    -          -            ");
            c14.push_back("        -     -       - -     -         -             ");
            c14.push_back("       -      W       W-      W        -              ");
            c14.push_back("              -        ------W        -               ");
            c14.push_back("");
            content[14] = c14;

            std::vector<std::string> c15;
            c15.push_back("");
            c15.push_back("              \\        \\\\\\\\\\WW        \\\\\\\\\\\\WW\\       ");
            c15.push_back("             W\\       \\      W\\              W        ");
            c15.push_back("            W \\       \\     W \\             W         ");
            c15.push_back("           \\  \\       \\    \\  \\            \\          ");
            c15.push_back("          \\   \\       \\   \\   \\        \\\\\\\\\\WW        ");
            c15.push_back("         \\    \\       \\  \\    \\          \\            ");
            c15.push_back("        \\     W       W \\     W         \\             ");
            c15.push_back("       \\      W       W\\      W        \\              ");
            c15.push_back("              \\        \\\\\\\\\\WW        \\               ");
            c15.push_back("");
            content[15] = c15;

            std::vector<std::string> c16;
            c16.push_back("");
            c16.push_back("              |        ||||WWG        |||||WWG|       ");
            c16.push_back("             G|       |      G|              G        ");
            c16.push_back("            W |       |     W |             W         ");
            c16.push_back("           W  |       |    W  |            W          ");
            c16.push_back("          |   |       |   |   |        ||||WWG        ");
            c16.push_back("         |    W       W  |    W          |            ");
            c16.push_back("        |     W       W |     W         |             ");
            c16.push_back("       |      G       G|      G        |              ");
            c16.push_back("              |        ||||WWG        |               ");
            c16.push_back("");
            content[16] = c16;

            std::vector<std::string> c17;
            c17.push_back("");
            c17.push_back("              /        ///WWG1        ////WWG1/       ");
            c17.push_back("             1/       /      1/              1        ");
            c17.push_back("            G /       /     G /             G         ");
            c17.push_back("           W  /       /    W  /            W          ");
            c17.push_back("          W   W       W   W   W        ///WWG1        ");
            c17.push_back("         /    W       W  /    W          /            ");
            c17.push_back("        /     G       G /     G         /             ");
            c17.push_back("       /      1       1/      1        /              ");
            c17.push_back("              /        ///WWG1        /               ");
            c17.push_back("");
            content[17] = c17;

            std::vector<std::string> c18;
            c18.push_back("");
            c18.push_back("              -        --WWG1W        ---WWG1W-       ");
            c18.push_back("             W-       -      W-              W        ");
            c18.push_back("            1 -       -     1 -             1         ");
            c18.push_back("           G  W       W    G  W            G          ");
            c18.push_back("          W   W       W   W   W        --WWG1W        ");
            c18.push_back("         W    G       G  W    G          W            ");
            c18.push_back("        -     1       1 -     1         -             ");
            c18.push_back("       -      W       W-      W        -              ");
            c18.push_back("              -        --WWG1W        -               ");
            c18.push_back("");
            content[18] = c18;

            std::vector<std::string> c19;
            c19.push_back("");
            c19.push_back("              \\        \\WWG1WG        \\\\WWG1WG\\       ");
            c19.push_back("             G\\       \\      G\\              G        ");
            c19.push_back("            W W       W     W W             W         ");
            c19.push_back("           1  W       W    1  W            1          ");
            c19.push_back("          G   G       G   G   G        \\WWG1WG        ");
            c19.push_back("         W    1       1  W    1          W            ");
            c19.push_back("        W     W       W W     W         W             ");
            c19.push_back("       \\      G       G\\      G        \\              ");
            c19.push_back("              \\        \\WWG1WG        \\               ");
            c19.push_back("");
            content[19] = c19;

            return content;
        }();

    int busy_index = 0;
    bool finished = false;
    auto start = std::chrono::high_resolution_clock::now();

    // clear old window content
    for (int i = 1; i < height - 1; ++i)
        for (int j = 1; j < width - 1; ++j)
            mvwaddch(w, i, j, ' ');

    std::thread animation_thread{
        [&]()
        {
            while (!finished)
            {
                auto stop = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
                if (duration.count() > 107)
                {
                    int x_margin = width / 2 - busy_content[busy_index][1].length() / 2;
                    int y_margin = height / 2 - busy_content[busy_index].size() / 2;
                    --y_margin;

                    for (int line = 0; line < (int) busy_content[busy_index].size(); ++line)
                        mvwprintw(w, y_margin + line, x_margin, busy_content[busy_index][line].c_str());

                    ++busy_index;
                    if (busy_index >= (int) busy_content.size())
                        busy_index = 0;

                    wrefresh(w);

                    start = std::chrono::high_resolution_clock::now();
                }

                std::this_thread::sleep_for(std::chrono::microseconds(17));
            }
        }
    };

    // clear out the word stack
    while (!ws.empty())
        ws.pop();

    // clear out completion stack
    while (cs.count() > 1)
        cs.pop();
    cs.clear_top();

    finished = true;

    input_win.mark_dirty();

    animation_thread.join();
}


void FilterWindow::draw_hook()
{
    int top_margin = height - (int) ((height / 1.618 + word_attribute::variants().size() / 2.0) + 0.5);
    int indent = width - (int) (width / 1.618 + 0.5);

    static std::string const filter_type{"filter type: "};

    mvwprintw(w, top_margin - 2, indent - filter_type.length(), filter_type.c_str());

    if (hover == -1)
        wattron(w, A_BOLD);
    mvwprintw(w, top_margin - 2, indent, wf.direction.as_string().c_str());
    if (hover == -1)
        wattroff(w, A_BOLD);

    int i = 0;
    for (; i < (int) word_attribute::variants().size() && i + top_margin < height - 2; ++i)
    {
        auto att = word_attribute::from_index(i);
        std::string const & att_str = att.as_string();

        if (wf.attributes.is_set(att))
            wattron(w, A_REVERSE);

        if (hover == i)
            wattron(w, A_BOLD);

        int j = 0;
        for (; j < (int) att_str.length() && j + indent < width - 2; ++j)
            mvwaddch(w, i + top_margin, j + indent, att_str[j]);

        if (wf.attributes.is_set(att))
            wattroff(w, A_REVERSE);

        if (hover == i)
            wattroff(w, A_BOLD);

        // blank out rest of line
        for (; j + indent < width - 2; ++j)
            mvwaddch(w, i + top_margin, j + indent, ' ');
    }

    // blank out remaining lines
    for (; i + top_margin < height - 2; ++i)
        for (int j = 0; j < width - 2; ++j)
            mvwaddch(w, i + top_margin, j + 1, ' ');
}


void FilterWindow::on_KEY_UP()
{
    if (hover > -1)
    {
        --hover;
        mark_dirty();
    }
}


void FilterWindow::on_KEY_DOWN()
{
    if (hover < (int) word_attribute::variants().size() - 1)
    {
        ++hover;
        mark_dirty();
    }
}


void FilterWindow::on_RETURN()
{
    // first option in filter window is exclusive / inclusive,
    // for which we use -1 because it is both before and outside the index range for word_attribute
    if (hover == -1)
    {
        int direction_index = wf.direction.as_index() + 1;
        if (direction_index >= (int) filter_direction::variants().size())
            direction_index = 0;
        wf.direction = filter_direction::from_index(direction_index);
    }
    else if (hover >= 0)
    {
        wf.attributes.toggle(word_attribute::from_index(hover));
    }
    else
    {
        std::cerr << "FilterWindow::toggle_hovered() :  hover [" << hover
                  << "] is outside valid range [-1.." << word_attribute::variants().size() << "]"
                  << std::endl;
    }

    mark_dirty();
}


Layer::Type FilterWindow::layer() const
{
    return Layer::F::grab();
}
