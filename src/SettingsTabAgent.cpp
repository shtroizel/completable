#include "SettingsTabAgent.h"

#include "IndicatorWindow.h"
#include "TabDescriptionWindow.h"
#include "AccessHelpWindow.h"
#include "SettingsHelpWindow.h"
#include "SettingsTab.h"
#include "SettingsWindow.h"


SettingsTabAgent::SettingsTabAgent(
    std::shared_ptr<TabDescriptionWindow> pdw,
    std::shared_ptr<IndicatorWindow> iw
)
    : tab_desc_win{pdw}
    , indicator_win{iw}
    , access_help_win{std::make_shared<AccessHelpWindow>()}
    , help_win{std::make_shared<SettingsHelpWindow>()}
    , settings_win{std::make_shared<SettingsWindow>()}
    , settings_tab{std::make_shared<SettingsTab>()}
{
    settings_tab->add_window(tab_desc_win.get());
    settings_tab->add_window(indicator_win.get());

    settings_tab->add_window(access_help_win.get());
    settings_tab->add_window(help_win.get());
    settings_tab->add_window(settings_win.get());
    settings_tab->set_active_window(settings_win.get());
}
