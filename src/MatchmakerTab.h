#pragma once

#include "AbstractTab.h"



// create Tab variant, "matchmaker" as a handle to MatchmakerTab
GROW_MATCHABLE(Tab, matchmaker)

/**
 * MatchmakerTab manages windows used for the dynamic loading of matchmaker dictionaries
 */
class MatchmakerTab : public AbstractTab
{
public:
    MatchmakerTab();

private:
    // resolved dependencies
    int indicator_position() const final { return 2; }
    Tab::Type as_matchable() const final { return Tab::matchmaker::grab(); }
};
