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



#include "MatchmakerTab.h"



MatchmakerTab::MatchmakerTab() : AbstractTab()
{
    Tab::matchmaker::grab().set_AbstractTab(this);
}


std::array<char, 17> const & MatchmakerTab::description() const
{
    static std::array<char, 17> desc = []()
                                       {
                                           std::array<char, 17> d;
                                           d[0]  = 'm';
                                           d[1]  = 'a';
                                           d[2]  = 't';
                                           d[3]  = 'c';
                                           d[4]  = 'h';
                                           d[5]  = 'm';
                                           d[6]  = 'a';
                                           d[7]  = 'k';
                                           d[8]  = 'e';
                                           d[9]  = 'r';
                                           d[10] = ' ';
                                           d[11] = ' ';
                                           d[12] = ' ';
                                           d[13] = ' ';
                                           d[14] = ' ';
                                           d[15] = ' ';
                                           d[16] = ' ';
                                           return d;
                                       }();
    return desc;
}
