#pragma once

#include <memory>
#include <stack>

#include "SettingsTab.h"



class IndicatorWindow;
class TabDescriptionWindow;
class AccessHelpWindow;
class SettingsHelpWindow;
class SettingsWindow;

/**
 * SettingsTabAgent constructs and provides access to SettingsTab
 */
class SettingsTabAgent
{
public:
    SettingsTabAgent(SettingsTabAgent const &) = delete;
    SettingsTabAgent & operator=(SettingsTabAgent const &) = delete;

    SettingsTabAgent(std::shared_ptr<TabDescriptionWindow>, std::shared_ptr<IndicatorWindow>);
    SettingsTab * operator()() { return settings_tab.get(); }

private:
    std::shared_ptr<TabDescriptionWindow> tab_desc_win;
    std::shared_ptr<IndicatorWindow> indicator_win;

    std::shared_ptr<AccessHelpWindow> access_help_win;
    std::shared_ptr<SettingsHelpWindow> help_win;
    std::shared_ptr<SettingsWindow> settings_win;
    std::shared_ptr<SettingsTab> settings_tab;
};
