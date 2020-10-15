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



#include "MatchmakerTabAgent.h"

#include "IndicatorWindow.h"
#include "TabDescriptionWindow.h"
#include "AccessHelpWindow.h"
#include "MatchmakerHelpWindow.h"
#include "MatchmakerTab.h"
#include "SettingsWindow.h"


MatchmakerTabAgent::MatchmakerTabAgent(
    std::shared_ptr<TabDescriptionWindow> pdw,
    std::shared_ptr<IndicatorWindow> iw
)
    : tab_desc_win{pdw}
    , indicator_win{iw}
    , access_help_win{std::make_shared<AccessHelpWindow>()}
    , help_win{std::make_shared<MatchmakerHelpWindow>()}
    , settings_win{std::make_shared<SettingsWindow>()}
    , matchmaker_tab{std::make_shared<MatchmakerTab>()}
{
    matchmaker_tab->add_window(tab_desc_win.get());
    matchmaker_tab->add_window(indicator_win.get());

    matchmaker_tab->add_window(access_help_win.get());
    matchmaker_tab->add_window(help_win.get());
    matchmaker_tab->set_active_window(settings_win.get());
}
