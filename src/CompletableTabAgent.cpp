#include "CompletableTabAgent.h"

#include "AntonymWindow.h"
#include "AttributeWindow.h"
#include "CompletableTab.h"
#include "CompletionStack.h"
#include "CompletionWindow.h"
#include "exec_long_task_with_busy_animation.h"
#include "FilterWindow.h"
#include "CompletableHelpWindow.h"
#include "IndicatorWindow.h"
#include "InputWindow.h"
#include "Layer.h"
#include "LengthCompletionWindow.h"
#include "MatchmakerState.h"
#include "OrdinalSummationWindow.h"
#include "TabDescriptionWindow.h"
#include "Settings.h"
#include "SynonymWindow.h"
#include "VisibilityAspect.h"
#include "word_filter.h"



CompletableTabAgent::CompletableTabAgent(
    std::shared_ptr<TabDescriptionWindow> pdw,
    std::shared_ptr<IndicatorWindow> iw
)
    : tab_desc_win{pdw}
    , indicator_win{iw}
    , wf{std::make_shared<word_filter>()}
    , cs{std::make_shared<CompletionStack>(*wf)}
    , ws{}
    , input_win{std::make_shared<InputWindow>(*cs, ws)}
    , completion_win{std::make_shared<CompletionWindow>(*cs, ws, *input_win, *wf)}
    , len_completion_win{std::make_shared<LengthCompletionWindow>(*cs, ws, *input_win, *wf, *completion_win)}
    , ord_sum_win{std::make_shared<OrdinalSummationWindow>(*cs, ws, *input_win, *wf, *completion_win)}
    , syn_win{std::make_shared<SynonymWindow>(*cs, ws, *input_win, *completion_win, *wf)}
    , ant_win{std::make_shared<AntonymWindow>(*cs, ws, *input_win, *completion_win, *syn_win, *wf)}
    , att_win{std::make_shared<AttributeWindow>(*cs, ws, *completion_win, *len_completion_win, *syn_win, *ant_win)}
    , filter_win{std::make_shared<FilterWindow>(*cs, ws, *input_win, *wf)}
    , completable_help_win{std::make_shared<CompletableHelpWindow>()}
    , completable_tab{std::make_shared<CompletableTab>()}
{
    filter_win->disable(VisibilityAspect::WindowVisibility::grab());

    completable_tab->add_window(tab_desc_win.get());
    completable_tab->add_window(indicator_win.get());
    completable_tab->add_window(input_win.get());
    completable_tab->add_window(completion_win.get());
    completable_tab->add_window(len_completion_win.get());
    completable_tab->add_window(ord_sum_win.get());
    completable_tab->add_window(att_win.get());
    completable_tab->add_window(syn_win.get());
    completable_tab->add_window(ant_win.get());
    completable_tab->add_window(filter_win.get());
    completable_tab->add_window(completable_help_win.get());

    completable_tab->set_active_window(completion_win.get());
    completable_tab->set_active_window(completable_help_win.get());

    // define neighbors for left/right arrow key switching of windows within completable tab
    completion_win->set_left_neighbor(Tab::completable::grab(), ant_win.get());
    completion_win->set_right_neighbor(Tab::completable::grab(), syn_win.get());
    len_completion_win->set_left_neighbor(Tab::completable::grab(), syn_win.get());
    len_completion_win->set_right_neighbor(Tab::completable::grab(), ord_sum_win.get());
    ord_sum_win->set_left_neighbor(Tab::completable::grab(), len_completion_win.get());
    ord_sum_win->set_right_neighbor(Tab::completable::grab(), ant_win.get());
    syn_win->set_left_neighbor(Tab::completable::grab(), completion_win.get());
    syn_win->set_right_neighbor(Tab::completable::grab(), len_completion_win.get());
    ant_win->set_left_neighbor(Tab::completable::grab(), ord_sum_win.get());
    ant_win->set_right_neighbor(Tab::completable::grab(), completion_win.get());

    // prevent both Length_spc_Completion and Ordinal_spc_Summation from being enabled simultaneously!
    EnablednessSetting::Length_spc_Completion::grab().add_enabledness_observer(
        [&]()
        {
            if (EnablednessSetting::Length_spc_Completion::grab().as_enabledness() ==
                    Enabledness::Enabled::grab())
            {
                EnablednessSetting::Ordinal_spc_Summation::grab().set_enabledness(
                        Enabledness::Disabled::grab());
            }
        }
    );
    EnablednessSetting::Ordinal_spc_Summation::grab().add_enabledness_observer(
        [&]()
        {
            if (EnablednessSetting::Ordinal_spc_Summation::grab().as_enabledness() ==
                    Enabledness::Enabled::grab())
            {
                EnablednessSetting::Length_spc_Completion::grab().set_enabledness(
                        Enabledness::Disabled::grab());
            }
        }
    );

    // initial enabledness
    completion_win->set_enabled(
        EnablednessSetting::CompletionList::grab().as_enabledness() == Enabledness::Enabled::grab(),
        VisibilityAspect::WindowVisibility::grab()
    );
    len_completion_win->set_enabled(
        EnablednessSetting::Length_spc_Completion::grab().as_enabledness() == Enabledness::Enabled::grab(),
        VisibilityAspect::WindowVisibility::grab()
    );
    ord_sum_win->set_enabled(
        EnablednessSetting::Ordinal_spc_Summation::grab().as_enabledness() == Enabledness::Enabled::grab(),
        VisibilityAspect::WindowVisibility::grab()
    );
    ant_win->set_enabled(
        EnablednessSetting::Antonyms::grab().as_enabledness() == Enabledness::Enabled::grab(),
        VisibilityAspect::WindowVisibility::grab()
    );

    // response to window enabledness changes
    auto on_win_enabledness =
        [&](EnablednessSetting::Type setting, AbstractWindow * win)
        {
            setting.as_enabledness().match({
                {
                    Enabledness::Enabled::grab(),
                    [&]()
                    {
                        win->enable(VisibilityAspect::WindowVisibility::grab());
                    }
                },
                {
                    Enabledness::Disabled::grab(),
                    [&]()
                    {
                        if (completable_tab->get_active_window(Layer::Bottom::grab()) == win)
                            win->activate_left(Tab::completable::grab());

                        win->disable(VisibilityAspect::WindowVisibility::grab());
                    }
                },
            });

            // resize all other windows
            AbstractWindow * start = win->get_left_neighbor(Tab::completable::grab());
            AbstractWindow * iter = start;
            do
            {
                iter->resize();
                iter = iter->get_left_neighbor(Tab::completable::grab());
            }
            while (iter != start);
        };

    EnablednessSetting::Length_spc_Completion::grab().add_enabledness_observer(
        std::bind(
            on_win_enabledness,
            EnablednessSetting::Length_spc_Completion::grab(),
            len_completion_win.get()
        )
    );

    EnablednessSetting::Ordinal_spc_Summation::grab().add_enabledness_observer(
        std::bind(
            on_win_enabledness,
            EnablednessSetting::Ordinal_spc_Summation::grab(),
            ord_sum_win.get()
        )
    );

    EnablednessSetting::Antonyms::grab().add_enabledness_observer(
        std::bind(
            on_win_enabledness,
            EnablednessSetting::Antonyms::grab(),
            ant_win.get()
        )
    );

    // respond to matchmaker state changes
    MatchmakerState::Instance::grab().add_state_observer(
        [&]()
        {
            // clear out the word stack
            while (!ws.empty())
                ws.pop();

            // clear out completion stack
            cs->clear_all();

            if (MatchmakerState::Instance::grab().as_state() == LibraryState::Loaded::grab())
                AbstractTab::set_active_tab(Tab::completable::grab());
        }
    );
}
