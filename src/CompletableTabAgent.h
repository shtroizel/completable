#pragma once

#include <memory>
#include <stack>

#include "CompletableTab.h"



class AntonymWindow;
class AttributeWindow;
class CompletionStack;
class CompletionWindow;
class FilterWindow;
class CompletableHelpWindow;
class IndicatorWindow;
class InputWindow;
class LengthCompletionWindow;
class OrdinalSummationWindow;
class TabDescriptionWindow;
class SynonymWindow;

struct word_filter;
struct word_stack_element;


/**
 * CompletableTabAgent constructs and provides access to CompletableTab
 */
class CompletableTabAgent
{
public:
    CompletableTabAgent(CompletableTabAgent const &) = delete;
    CompletableTabAgent & operator=(CompletableTabAgent const &) = delete;

    CompletableTabAgent(std::shared_ptr<TabDescriptionWindow>, std::shared_ptr<IndicatorWindow>);
    CompletableTab * operator()() { return completable_tab.get(); }

private:
    std::shared_ptr<TabDescriptionWindow> tab_desc_win;
    std::shared_ptr<IndicatorWindow> indicator_win;

    std::shared_ptr<word_filter> wf;
    std::shared_ptr<CompletionStack> cs;
    std::stack<word_stack_element> ws;
    std::shared_ptr<InputWindow> input_win;
    std::shared_ptr<CompletionWindow> completion_win;
    std::shared_ptr<LengthCompletionWindow> len_completion_win;
    std::shared_ptr<OrdinalSummationWindow> ord_sum_win;
    std::shared_ptr<SynonymWindow> syn_win;
    std::shared_ptr<AntonymWindow> ant_win;
    std::shared_ptr<AttributeWindow> att_win;
    std::shared_ptr<FilterWindow> filter_win;
    std::shared_ptr<CompletableHelpWindow> completable_help_win;
    std::shared_ptr<CompletableTab> completable_tab;
};
