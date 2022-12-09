#include "MatchmakerTab.h"



MatchmakerTab::MatchmakerTab() : AbstractTab()
{
    Tab::matchmaker::grab().set_AbstractTab(this);
}
