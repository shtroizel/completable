#include "MatchmakerTabAgent.h"

#include "IndicatorWindow.h"
#include "TabDescriptionWindow.h"
#include "AccessHelpWindow.h"
#include "MatchmakerHelpWindow.h"
#include "MatchmakerTab.h"
#include "MatchmakerLocationWindow.h"
#include "MatchmakerSelectionWindow.h"


MatchmakerTabAgent::MatchmakerTabAgent(
    std::shared_ptr<TabDescriptionWindow> pdw,
    std::shared_ptr<IndicatorWindow> iw
)
    : tab_desc_win{pdw}
    , indicator_win{iw}
    , access_help_win{std::make_shared<AccessHelpWindow>()}
    , help_win{std::make_shared<MatchmakerHelpWindow>()}
    , mm_sel_win{std::make_shared<MatchmakerSelectionWindow>()}
    , mm_loc_win{std::make_shared<MatchmakerLocationWindow>(*mm_sel_win)}
    , matchmaker_tab{std::make_shared<MatchmakerTab>()}
{
    matchmaker_tab->add_window(tab_desc_win.get());
    matchmaker_tab->add_window(indicator_win.get());
    matchmaker_tab->add_window(access_help_win.get());
    matchmaker_tab->add_window(mm_loc_win.get());
    matchmaker_tab->add_window(mm_sel_win.get());
    matchmaker_tab->add_window(help_win.get());
    matchmaker_tab->set_active_window(mm_loc_win.get());
}
