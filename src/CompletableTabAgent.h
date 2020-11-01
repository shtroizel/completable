#pragma once

/*
Copyright (c) 2020, Eric Hyer
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/



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
    std::shared_ptr<SynonymWindow> syn_win;
    std::shared_ptr<AntonymWindow> ant_win;
    std::shared_ptr<AttributeWindow> att_win;
    std::shared_ptr<FilterWindow> filter_win;
    std::shared_ptr<CompletableHelpWindow> completable_help_win;
    std::shared_ptr<CompletableTab> completable_tab;
};
