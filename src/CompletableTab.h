#pragma once

#include <string>
#include <stack>
#include <vector>

#include "AbstractTab.h"



// create Tab variant, "completable" as a handle to CompletableTab
GROW_MATCHABLE(Tab, completable)

/**
 * CompletableTab manages windows used for the application's core functionality
 */
class CompletableTab : public AbstractTab
{
public:
    CompletableTab();

private:
    // resolved dependencies
    int indicator_position() const final { return 0; }
    Tab::Type as_matchable() const final { return Tab::completable::grab(); }
};
