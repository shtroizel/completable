#pragma once

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
