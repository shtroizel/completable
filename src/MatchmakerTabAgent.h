#pragma once

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



#include <memory>
#include <stack>

#include "MatchmakerTab.h"



class IndicatorWindow;
class TabDescriptionWindow;
class AccessHelpWindow;
class MatchmakerHelpWindow;
class MatchmakerLocationWindow;
class MatchmakerSelectionWindow;

/**
 * MatchmakerTabAgent constructs and provides access to MatchmakerTab
 */
class MatchmakerTabAgent
{
public:
    MatchmakerTabAgent(MatchmakerTabAgent const &) = delete;
    MatchmakerTabAgent & operator=(MatchmakerTabAgent const &) = delete;

    MatchmakerTabAgent(std::shared_ptr<TabDescriptionWindow>, std::shared_ptr<IndicatorWindow>);
    MatchmakerTab * operator()() { return matchmaker_tab.get(); }

private:
    std::shared_ptr<TabDescriptionWindow> tab_desc_win;
    std::shared_ptr<IndicatorWindow> indicator_win;

    std::shared_ptr<AccessHelpWindow> access_help_win;
    std::shared_ptr<MatchmakerHelpWindow> help_win;
    std::shared_ptr<MatchmakerSelectionWindow> mm_sel_win;
    std::shared_ptr<MatchmakerLocationWindow> mm_loc_win;
    std::shared_ptr<MatchmakerTab> matchmaker_tab;
};
