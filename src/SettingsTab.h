#pragma once

#include "AbstractTab.h"



// create Tab variant, "settings" as a handle to SettingsTab
GROW_MATCHABLE(Tab, settings)

/**
 * SettingsTab manages windows used for editing the application's settings
 */
class SettingsTab : public AbstractTab
{
public:
    SettingsTab();

private:
    // resolved dependencies
    int indicator_position() const final { return 1; }
    Tab::Type as_matchable() const final { return Tab::settings::grab(); }
};
