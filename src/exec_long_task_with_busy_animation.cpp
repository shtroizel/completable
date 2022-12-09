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



#include "exec_long_task_with_busy_animation.h"

#include <functional>
#include <iostream>
#include <thread>

#include <ncurses.h>

#include "AbstractWindow.h"
#include "Settings.h"



static std::array<std::vector<std::string>, 24> const one0seven =
    []()
    {
        std::array<std::vector<std::string>, 24> content;

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
        c1.push_back("              |        WWG1WGA        |WWG1WGA|       ");
        c1.push_back("             AW       W      AW              A        ");
        c1.push_back("            G W       W     G W             G         ");
        c1.push_back("           W  G       G    W  G            W          ");
        c1.push_back("          1   1       1   1   1        WWG1WGA        ");
        c1.push_back("         G    W       W  G    W          G            ");
        c1.push_back("        W     G       G W     G         W             ");
        c1.push_back("       W      A       AW      A        W              ");
        c1.push_back("              |        WWG1WGA        |               ");
        c1.push_back("");
        content[1] = c1;

        std::vector<std::string> c2;
        c2.push_back("");
        c2.push_back("              |        WWG1WGA        |WWG1WGA|       ");
        c2.push_back("             AW       W      AW              A        ");
        c2.push_back("            G W       W     G W             G         ");
        c2.push_back("           W  G       G    W  G            W          ");
        c2.push_back("          1   1       1   1   1        WWG1WGA        ");
        c2.push_back("         G    W       W  G    W          G            ");
        c2.push_back("        W     G       G W     G         W             ");
        c2.push_back("       W      A       AW      A        W              ");
        c2.push_back("              |        WWG1WGA        |               ");
        c2.push_back("");
        content[2] = c2;

        std::vector<std::string> c3;
        c3.push_back("");
        c3.push_back("              /        WWG1WGA        /WWG1WGA/       ");
        c3.push_back("             AW       W      AW              A        ");
        c3.push_back("            G W       W     G W             G         ");
        c3.push_back("           W  G       G    W  G            W          ");
        c3.push_back("          1   1       1   1   1        WWG1WGA        ");
        c3.push_back("         G    W       W  G    W          G            ");
        c3.push_back("        W     G       G W     G         W             ");
        c3.push_back("       W      A       AW      A        W              ");
        c3.push_back("              /        WWG1WGA        /               ");
        c3.push_back("");
        content[3] = c3;

        std::vector<std::string> c4;
        c4.push_back("");
        c4.push_back("              /        WWG1WGA        /WWG1WGA/       ");
        c4.push_back("             AW       W      AW              A        ");
        c4.push_back("            G W       W     G W             G         ");
        c4.push_back("           W  G       G    W  G            W          ");
        c4.push_back("          1   1       1   1   1        WWG1WGA        ");
        c4.push_back("         G    W       W  G    W          G            ");
        c4.push_back("        W     G       G W     G         W             ");
        c4.push_back("       W      A       AW      A        W              ");
        c4.push_back("              /        WWG1WGA        /               ");
        c4.push_back("");
        content[4] = c4;

        std::vector<std::string> c5;
        c5.push_back("");
        c5.push_back("              /        WWG1WGA        /WWG1WGA/       ");
        c5.push_back("             AW       W      AW              A        ");
        c5.push_back("            G W       W     G W             G         ");
        c5.push_back("           W  G       G    W  G            W          ");
        c5.push_back("          1   1       1   1   1        WWG1WGA        ");
        c5.push_back("         G    W       W  G    W          G            ");
        c5.push_back("        W     G       G W     G         W             ");
        c5.push_back("       W      A       AW      A        W              ");
        c5.push_back("              /        WWG1WGA        /               ");
        c5.push_back("");
        content[5] = c5;

        std::vector<std::string> c6;
        c6.push_back("");
        c6.push_back("              -        WWG1WGA        -WWG1WGA-       ");
        c6.push_back("             AW       W      AW              A        ");
        c6.push_back("            G W       W     G W             G         ");
        c6.push_back("           W  G       G    W  G            W          ");
        c6.push_back("          1   1       1   1   1        WWG1WGA        ");
        c6.push_back("         G    W       W  G    W          G            ");
        c6.push_back("        W     G       G W     G         W             ");
        c6.push_back("       W      A       AW      A        W              ");
        c6.push_back("              -        WWG1WGA        -               ");
        c6.push_back("");
        content[6] = c6;

        std::vector<std::string> c7;
        c7.push_back("");
        c7.push_back("              -        WG7WGA-        -WG7WGA--       ");
        c7.push_back("             -W       W      -W              -        ");
        c7.push_back("            A G       G     A G             A         ");
        c7.push_back("           G  7       7    G  7            G          ");
        c7.push_back("          W   W       W   W   W        WG7WGA-        ");
        c7.push_back("         7    G       G  7    G          7            ");
        c7.push_back("        G     A       A G     A         G             ");
        c7.push_back("       W      -       -W      -        W              ");
        c7.push_back("              -        WG7WGA-        -               ");
        c7.push_back("");

        content[7] = c7;

        std::vector<std::string> c8;
        c8.push_back("");
        c8.push_back("              -        G1WGA--        -G1WGA---       ");
        c8.push_back("             -G       G      -G              -        ");
        c8.push_back("            - 1       1     - 1             -         ");
        c8.push_back("           A  W       W    A  W            A          ");
        c8.push_back("          G   G       G   G   G        G1WGA--        ");
        c8.push_back("         W    A       A  W    A          W            ");
        c8.push_back("        1     -       - 1     -         1             ");
        c8.push_back("       G      -       -G      -        G              ");
        c8.push_back("              -        G1WGA--        -               ");
        c8.push_back("");
        content[8] = c8;

        std::vector<std::string> c9;
        c9.push_back("");
        c9.push_back("              \\        1WGA\\\\\\        \\1WGA\\\\\\\\       ");
        c9.push_back("             \\1       1      \\1              \\        ");
        c9.push_back("            \\ W       W     \\ W             \\         ");
        c9.push_back("           \\  G       G    \\  G            \\          ");
        c9.push_back("          A   A       A   A   A        1WGA\\\\\\        ");
        c9.push_back("         G    \\       \\  G    \\          G            ");
        c9.push_back("        W     \\       \\ W     \\         W             ");
        c9.push_back("       1      \\       \\1      \\        1              ");
        c9.push_back("              \\        1WGA\\\\\\        \\               ");
        c9.push_back("");
        content[9] = c9;

        std::vector<std::string> c10;
        c10.push_back("");
        c10.push_back("              \\        WGA\\\\\\\\        \\WGA\\\\\\\\\\       ");
        c10.push_back("             \\W       W      \\W              \\        ");
        c10.push_back("            \\ G       G     \\ G             \\         ");
        c10.push_back("           \\  A       A    \\  A            \\          ");
        c10.push_back("          \\   \\       \\   \\   \\        WGA\\\\\\\\        ");
        c10.push_back("         A    \\       \\  A    \\          A            ");
        c10.push_back("        G     \\       \\ G     \\         G             ");
        c10.push_back("       W      \\       \\W      \\        W              ");
        c10.push_back("              \\        WGA\\\\\\\\        \\               ");
        c10.push_back("");
        content[10] = c10;

        std::vector<std::string> c11;
        c11.push_back("");
        c11.push_back("              \\        GA\\\\\\\\\\        \\GA\\\\\\\\\\\\       ");
        c11.push_back("             \\G       G      \\G              \\        ");
        c11.push_back("            \\ A       A     \\ A             \\         ");
        c11.push_back("           \\  \\       \\    \\  \\            \\          ");
        c11.push_back("          \\   \\       \\   \\   \\        GA\\\\\\\\\\        ");
        c11.push_back("         \\    \\       \\  \\    \\          \\            ");
        c11.push_back("        A     \\       \\ A     \\         A             ");
        c11.push_back("       G      \\       \\G      \\        G              ");
        c11.push_back("              \\        GA\\\\\\\\\\        \\               ");
        c11.push_back("");
        content[11] = c11;

        std::vector<std::string> c12;
        c12.push_back("");
        c12.push_back("              |        A||||||        |A|||||||       ");
        c12.push_back("             |A       A      |A              |        ");
        c12.push_back("            | |       |     | |             |         ");
        c12.push_back("           |  |       |    |  |            |          ");
        c12.push_back("          |   |       |   |   |        A||||||        ");
        c12.push_back("         |    |       |  |    |          |            ");
        c12.push_back("        |     |       | |     |         |             ");
        c12.push_back("       A      |       |A      |        A              ");
        c12.push_back("              |        A||||||        |               ");
        c12.push_back("");
        content[12] = c12;

        std::vector<std::string> c13;
        c13.push_back("");
        c13.push_back("              |        |||||||        |||||||||       ");
        c13.push_back("             ||       |      ||              |        ");
        c13.push_back("            | |       |     | |             |         ");
        c13.push_back("           |  |       |    |  |            |          ");
        c13.push_back("          |   |       |   |   |        |||||||        ");
        c13.push_back("         |    |       |  |    |          |            ");
        c13.push_back("        |     |       | |     |         |             ");
        c13.push_back("       |      |       ||      |        |              ");
        c13.push_back("              |        |||||||        |               ");
        c13.push_back("");
        content[13] = c13;

        std::vector<std::string> c14;
        c14.push_back("");
        c14.push_back("              |        |||||||        |||||||||       ");
        c14.push_back("             ||       |      ||              |        ");
        c14.push_back("            | |       |     | |             |         ");
        c14.push_back("           |  |       |    |  |            |          ");
        c14.push_back("          |   |       |   |   |        |||||||        ");
        c14.push_back("         |    |       |  |    |          |            ");
        c14.push_back("        |     |       | |     |         |             ");
        c14.push_back("       |      |       ||      |        |              ");
        c14.push_back("              |        |||||||        |               ");
        c14.push_back("");
        content[14] = c14;

        std::vector<std::string> c15;
        c15.push_back("");
        c15.push_back("              /        ///////        /////////       ");
        c15.push_back("             //       /      //              /        ");
        c15.push_back("            / /       /     / /             /         ");
        c15.push_back("           /  /       /    /  /            /          ");
        c15.push_back("          /   /       /   /   /        ///////        ");
        c15.push_back("         /    /       /  /    /          /            ");
        c15.push_back("        /     /       / /     /         /             ");
        c15.push_back("       /      /       //      /        /              ");
        c15.push_back("              /        ///////        /               ");
        c15.push_back("");
        content[15] = c15;

        std::vector<std::string> c16;
        c16.push_back("");
        c16.push_back("              /        ///////        /////////       ");
        c16.push_back("             //       /      //              /        ");
        c16.push_back("            / /       /     / /             /         ");
        c16.push_back("           /  /       /    /  /            /          ");
        c16.push_back("          /   /       /   /   /        ///////        ");
        c16.push_back("         /    /       /  /    /          /            ");
        c16.push_back("        /     /       / /     /         /             ");
        c16.push_back("       /      /       //      /        /              ");
        c16.push_back("              /        ///////        /               ");
        c16.push_back("");
        content[16] = c16;

        std::vector<std::string> c17;
        c17.push_back("");
        c17.push_back("              /        ///////        /////////       ");
        c17.push_back("             //       /      //              /        ");
        c17.push_back("            / /       /     / /             /         ");
        c17.push_back("           /  /       /    /  /            /          ");
        c17.push_back("          /   /       /   /   /        ///////        ");
        c17.push_back("         /    /       /  /    /          /            ");
        c17.push_back("        /     /       / /     /         /             ");
        c17.push_back("       /      /       //      /        /              ");
        c17.push_back("              /        ///////        /               ");
        c17.push_back("");
        content[17] = c17;

        std::vector<std::string> c18;
        c18.push_back("");
        c18.push_back("              -        ------W        -------W-       ");
        c18.push_back("             W-       -      W-              W        ");
        c18.push_back("            - -       -     - -             -         ");
        c18.push_back("           -  -       -    -  -            -          ");
        c18.push_back("          -   -       -   -   -        ------W        ");
        c18.push_back("         -    -       -  -    -          -            ");
        c18.push_back("        -     -       - -     -         -             ");
        c18.push_back("       -      W       W-      W        -              ");
        c18.push_back("              -        ------W        -               ");
        c18.push_back("");
        content[18] = c18;

        std::vector<std::string> c19;
        c19.push_back("");
        c19.push_back("              -        -----WW        ------WW-       ");
        c19.push_back("             W-       -      W-              W        ");
        c19.push_back("            W -       -     W -             W         ");
        c19.push_back("           -  -       -    -  -            -          ");
        c19.push_back("          -   -       -   -   -        -----WW        ");
        c19.push_back("         -    -       -  -    -          -            ");
        c19.push_back("        -     W       W -     W         -             ");
        c19.push_back("       -      W       W-      W        -              ");
        c19.push_back("              -        -----WW        -               ");
        c19.push_back("");
        content[19] = c19;

        std::vector<std::string> c20;
        c20.push_back("");
        c20.push_back("              -        ----WWG        -----WWG-       ");
        c20.push_back("             G-       -      G-              G        ");
        c20.push_back("            W -       -     W -             W         ");
        c20.push_back("           W  -       -    W  -            W          ");
        c20.push_back("          -   -       -   -   -        ----WWG        ");
        c20.push_back("         -    W       W  -    W          -            ");
        c20.push_back("        -     W       W -     W         -             ");
        c20.push_back("       -      G       G-      G        -              ");
        c20.push_back("              -        ----WWG        -               ");
        c20.push_back("");
        content[20] = c20;

        std::vector<std::string> c21;
        c21.push_back("");
        c21.push_back("              \\        \\\\\\WWG1        \\\\\\\\WWG1\\       ");
        c21.push_back("             1\\       \\      1\\              1        ");
        c21.push_back("            G \\       \\     G \\             G         ");
        c21.push_back("           W  \\       \\    W  \\            W          ");
        c21.push_back("          W   W       W   W   W        \\\\\\WWG1        ");
        c21.push_back("         \\    W       W  \\    W          \\            ");
        c21.push_back("        \\     G       G \\     G         \\             ");
        c21.push_back("       \\      1       1\\      1        \\              ");
        c21.push_back("              \\        \\\\\\WWG1        \\               ");
        c21.push_back("");
        content[21] = c21;

        std::vector<std::string> c22;
        c22.push_back("");
        c22.push_back("              \\        \\\\WWG1W        \\\\\\WWG1W\\       ");
        c22.push_back("             W\\       \\      W\\              W        ");
        c22.push_back("            1 \\       \\     1 \\             1         ");
        c22.push_back("           G  W       W    G  W            G          ");
        c22.push_back("          W   W       W   W   W        \\\\WWG1W        ");
        c22.push_back("         W    G       G  W    G          W            ");
        c22.push_back("        \\     1       1 \\     1         \\             ");
        c22.push_back("       \\      W       W\\      W        \\              ");
        c22.push_back("              \\        \\\\WWG1W        \\               ");
        c22.push_back("");
        content[22] = c22;

        std::vector<std::string> c23;
        c23.push_back("");
        c23.push_back("              \\        \\WWG1WG        \\\\WWG1WG\\       ");
        c23.push_back("             G\\       \\      G\\              G        ");
        c23.push_back("            W W       W     W W             W         ");
        c23.push_back("           1  W       W    1  W            1          ");
        c23.push_back("          G   G       G   G   G        \\WWG1WG        ");
        c23.push_back("         W    1       1  W    1          W            ");
        c23.push_back("        W     W       W W     W         W             ");
        c23.push_back("       \\      G       G\\      G        \\              ");
        c23.push_back("              \\        \\WWG1WG        \\               ");
        c23.push_back("");
        content[23] = c23;

        return content;
    }();


static std::array<std::vector<std::string>, 24> const simple_content =
    []()
    {
        std::array<std::vector<std::string>, 24> content;

        std::vector<std::string> c0;
        c0.push_back("");
        c0.push_back("                                                      ");
        c0.push_back("                                                      ");
        c0.push_back("                                                      ");
        c0.push_back("                                                      ");
        c0.push_back("                          |                           ");
        c0.push_back("                                                      ");
        c0.push_back("                                                      ");
        c0.push_back("                                                      ");
        c0.push_back("                                                      ");
        c0.push_back("");
        content[0] = c0;

        std::vector<std::string> c1;
        c1.push_back("");
        c1.push_back("                                                      ");
        c1.push_back("                                                      ");
        c1.push_back("                                                      ");
        c1.push_back("                                                      ");
        c1.push_back("                          |                           ");
        c1.push_back("                                                      ");
        c1.push_back("                                                      ");
        c1.push_back("                                                      ");
        c1.push_back("                                                      ");
        c1.push_back("");
        content[1] = c1;

        std::vector<std::string> c2;
        c2.push_back("");
        c2.push_back("                                                      ");
        c2.push_back("                                                      ");
        c2.push_back("                                                      ");
        c2.push_back("                                                      ");
        c2.push_back("                          |                           ");
        c2.push_back("                                                      ");
        c2.push_back("                                                      ");
        c2.push_back("                                                      ");
        c2.push_back("                                                      ");
        c2.push_back("");
        content[2] = c2;

        std::vector<std::string> c3;
        c3.push_back("");
        c3.push_back("                                                      ");
        c3.push_back("                                                      ");
        c3.push_back("                                                      ");
        c3.push_back("                                                      ");
        c3.push_back("                          /                           ");
        c3.push_back("                                                      ");
        c3.push_back("                                                      ");
        c3.push_back("                                                      ");
        c3.push_back("                                                      ");
        c3.push_back("");
        content[3] = c3;

        std::vector<std::string> c4;
        c4.push_back("");
        c4.push_back("                                                      ");
        c4.push_back("                                                      ");
        c4.push_back("                                                      ");
        c4.push_back("                                                      ");
        c4.push_back("                          /                           ");
        c4.push_back("                                                      ");
        c4.push_back("                                                      ");
        c4.push_back("                                                      ");
        c4.push_back("                                                      ");
        c4.push_back("");
        content[4] = c4;

        std::vector<std::string> c5;
        c5.push_back("");
        c5.push_back("                                                      ");
        c5.push_back("                                                      ");
        c5.push_back("                                                      ");
        c5.push_back("                                                      ");
        c5.push_back("                          /                           ");
        c5.push_back("                                                      ");
        c5.push_back("                                                      ");
        c5.push_back("                                                      ");
        c5.push_back("                                                      ");
        c5.push_back("");
        content[5] = c5;

        std::vector<std::string> c6;
        c6.push_back("");
        c6.push_back("                                                      ");
        c6.push_back("                                                      ");
        c6.push_back("                                                      ");
        c6.push_back("                                                      ");
        c6.push_back("                          -                           ");
        c6.push_back("                                                      ");
        c6.push_back("                                                      ");
        c6.push_back("                                                      ");
        c6.push_back("                                                      ");
        c6.push_back("");
        content[6] = c6;

        std::vector<std::string> c7;
        c7.push_back("");
        c7.push_back("                                                      ");
        c7.push_back("                                                      ");
        c7.push_back("                                                      ");
        c7.push_back("                                                      ");
        c7.push_back("                          -                           ");
        c7.push_back("                                                      ");
        c7.push_back("                                                      ");
        c7.push_back("                                                      ");
        c7.push_back("                                                      ");
        c7.push_back("");
        content[7] = c7;

        std::vector<std::string> c8;
        c8.push_back("");
        c8.push_back("                                                      ");
        c8.push_back("                                                      ");
        c8.push_back("                                                      ");
        c8.push_back("                                                      ");
        c8.push_back("                          -                           ");
        c8.push_back("                                                      ");
        c8.push_back("                                                      ");
        c8.push_back("                                                      ");
        c8.push_back("                                                      ");
        c8.push_back("");
        content[8] = c8;

        std::vector<std::string> c9;
        c9.push_back("");
        c9.push_back("                                                      ");
        c9.push_back("                                                      ");
        c9.push_back("                                                      ");
        c9.push_back("                                                      ");
        c9.push_back("                          \\                           ");
        c9.push_back("                                                      ");
        c9.push_back("                                                      ");
        c9.push_back("                                                      ");
        c9.push_back("                                                      ");
        c9.push_back("");
        content[9] = c9;

        std::vector<std::string> c10;
        c10.push_back("");
        c10.push_back("                                                      ");
        c10.push_back("                                                      ");
        c10.push_back("                                                      ");
        c10.push_back("                                                      ");
        c10.push_back("                          \\                           ");
        c10.push_back("                                                      ");
        c10.push_back("                                                      ");
        c10.push_back("                                                      ");
        c10.push_back("                                                      ");
        c10.push_back("");
        content[10] = c10;

        std::vector<std::string> c11;
        c11.push_back("");
        c11.push_back("                                                      ");
        c11.push_back("                                                      ");
        c11.push_back("                                                      ");
        c11.push_back("                                                      ");
        c11.push_back("                          \\                           ");
        c11.push_back("                                                      ");
        c11.push_back("                                                      ");
        c11.push_back("                                                      ");
        c11.push_back("                                                      ");
        c11.push_back("");
        content[11] = c11;

        std::vector<std::string> c12;
        c12.push_back("");
        c12.push_back("                                                      ");
        c12.push_back("                                                      ");
        c12.push_back("                                                      ");
        c12.push_back("                                                      ");
        c12.push_back("                          |                           ");
        c12.push_back("                                                      ");
        c12.push_back("                                                      ");
        c12.push_back("                                                      ");
        c12.push_back("                                                      ");
        c12.push_back("");
        content[12] = c12;

        std::vector<std::string> c13;
        c13.push_back("");
        c13.push_back("                                                      ");
        c13.push_back("                                                      ");
        c13.push_back("                                                      ");
        c13.push_back("                                                      ");
        c13.push_back("                          |                           ");
        c13.push_back("                                                      ");
        c13.push_back("                                                      ");
        c13.push_back("                                                      ");
        c13.push_back("                                                      ");
        c13.push_back("");
        content[13] = c13;

        std::vector<std::string> c14;
        c14.push_back("");
        c14.push_back("                                                      ");
        c14.push_back("                                                      ");
        c14.push_back("                                                      ");
        c14.push_back("                                                      ");
        c14.push_back("                          |                           ");
        c14.push_back("                                                      ");
        c14.push_back("                                                      ");
        c14.push_back("                                                      ");
        c14.push_back("                                                      ");
        c14.push_back("");
        content[14] = c14;

        std::vector<std::string> c15;
        c15.push_back("");
        c15.push_back("                                                      ");
        c15.push_back("                                                      ");
        c15.push_back("                                                      ");
        c15.push_back("                                                      ");
        c15.push_back("                          /                           ");
        c15.push_back("                                                      ");
        c15.push_back("                                                      ");
        c15.push_back("                                                      ");
        c15.push_back("                                                      ");
        c15.push_back("");
        content[15] = c15;

        std::vector<std::string> c16;
        c16.push_back("");
        c16.push_back("                                                      ");
        c16.push_back("                                                      ");
        c16.push_back("                                                      ");
        c16.push_back("                                                      ");
        c16.push_back("                          /                           ");
        c16.push_back("                                                      ");
        c16.push_back("                                                      ");
        c16.push_back("                                                      ");
        c16.push_back("                                                      ");
        c16.push_back("");
        content[16] = c16;

        std::vector<std::string> c17;
        c17.push_back("");
        c17.push_back("                                                      ");
        c17.push_back("                                                      ");
        c17.push_back("                                                      ");
        c17.push_back("                                                      ");
        c17.push_back("                          /                           ");
        c17.push_back("                                                      ");
        c17.push_back("                                                      ");
        c17.push_back("                                                      ");
        c17.push_back("                                                      ");
        c17.push_back("");
        content[17] = c17;

        std::vector<std::string> c18;
        c18.push_back("");
        c18.push_back("                                                      ");
        c18.push_back("                                                      ");
        c18.push_back("                                                      ");
        c18.push_back("                                                      ");
        c18.push_back("                          -                           ");
        c18.push_back("                                                      ");
        c18.push_back("                                                      ");
        c18.push_back("                                                      ");
        c18.push_back("                                                      ");
        c18.push_back("");
        content[18] = c18;

        std::vector<std::string> c19;
        c19.push_back("");
        c19.push_back("                                                      ");
        c19.push_back("                                                      ");
        c19.push_back("                                                      ");
        c19.push_back("                                                      ");
        c19.push_back("                          -                           ");
        c19.push_back("                                                      ");
        c19.push_back("                                                      ");
        c19.push_back("                                                      ");
        c19.push_back("                                                      ");
        c19.push_back("");
        content[19] = c19;

        std::vector<std::string> c20;
        c20.push_back("");
        c20.push_back("                                                      ");
        c20.push_back("                                                      ");
        c20.push_back("                                                      ");
        c20.push_back("                                                      ");
        c20.push_back("                          -                           ");
        c20.push_back("                                                      ");
        c20.push_back("                                                      ");
        c20.push_back("                                                      ");
        c20.push_back("                                                      ");
        c20.push_back("");
        content[20] = c20;

        std::vector<std::string> c21;
        c21.push_back("");
        c21.push_back("                                                      ");
        c21.push_back("                                                      ");
        c21.push_back("                                                      ");
        c21.push_back("                                                      ");
        c21.push_back("                          \\                           ");
        c21.push_back("                                                      ");
        c21.push_back("                                                      ");
        c21.push_back("                                                      ");
        c21.push_back("                                                      ");
        c21.push_back("");
        content[21] = c21;

        std::vector<std::string> c22;
        c22.push_back("");
        c22.push_back("                                                      ");
        c22.push_back("                                                      ");
        c22.push_back("                                                      ");
        c22.push_back("                                                      ");
        c22.push_back("                          \\                           ");
        c22.push_back("                                                      ");
        c22.push_back("                                                      ");
        c22.push_back("                                                      ");
        c22.push_back("                                                      ");
        c22.push_back("");
        content[22] = c22;

        std::vector<std::string> c23;
        c23.push_back("");
        c23.push_back("                                                      ");
        c23.push_back("                                                      ");
        c23.push_back("                                                      ");
        c23.push_back("                                                      ");
        c23.push_back("                          \\                           ");
        c23.push_back("                                                      ");
        c23.push_back("                                                      ");
        c23.push_back("                                                      ");
        c23.push_back("                                                      ");
        c23.push_back("");
        content[23] = c23;

        return content;
    }();



MATCHABLE_NIL_PROPERTY_VALUE(Animation, content, &simple_content)
MATCHABLE_VARIANT_PROPERTY_VALUE(Animation, esc_Default, content, &simple_content)
MATCHABLE_VARIANT_PROPERTY_VALUE(Animation, esc_Cheers_spc_To_spc_107, content, &one0seven)


void exec_long_task_with_busy_animation(std::function<void()> task, AbstractWindow & win)
{
    animation_content content = AnimationSetting::Busy_spc_Animation::grab().as_animation().as_content();
    if (nullptr == content)
    {
        std::cerr << "exec_long_task_with_busy_animation() --> animation content null for "
                  << AnimationSetting::Busy_spc_Animation::grab().as_animation() << std::endl;
        return;
    }
    int busy_index = 0;
    bool finished = false;
    auto start = std::chrono::high_resolution_clock::now();

    // clear old window content
    for (int i = 1; i < win.get_height() - 1; ++i)
        for (int j = 1; j < win.get_width() - 1; ++j)
            mvwaddch(win.get_WINDOW(), i, j, ' ');

    std::thread animation_thread{
        [&]()
        {
            while (!finished)
            {
                auto stop = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
                if (duration.count() > 124)
                {
                    int x_margin = win.get_width() / 2 - (*content)[busy_index][1].length() / 2;
                    int y_margin = win.get_height() / 2 - (*content)[busy_index].size() / 2;
                    --y_margin;

                    for (int line = 0; line < (int) (*content)[busy_index].size(); ++line)
                        mvwprintw(
                            win.get_WINDOW(),
                            y_margin + line,
                            x_margin,
                            "%s", (*content)[busy_index][line].c_str()
                        );

                    ++busy_index;
                    if (busy_index >= (int) content->size())
                        busy_index = 0;

                    wrefresh(win.get_WINDOW());

                    start = std::chrono::high_resolution_clock::now();
                }

                std::this_thread::sleep_for(std::chrono::microseconds(17));
            }
        }
    };

    task();
    finished = true;
    animation_thread.join();
}
