#include "SettingsTab.h"



SettingsTab::SettingsTab() : AbstractTab()
{
    Tab::settings::grab().set_AbstractTab(this);
}
