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



#include "CompletablePageAgent.h"

#include "AntonymWindow.h"
#include "AttributeWindow.h"
#include "CompletablePage.h"
#include "CompletionStack.h"
#include "CompletionWindow.h"
#include "FilterWindow.h"
#include "CompletableHelpWindow.h"
#include "IndicatorWindow.h"
#include "InputWindow.h"
#include "LengthCompletionWindow.h"
#include "PageDescriptionWindow.h"
#include "SynonymWindow.h"
#include "VisibilityAspect.h"
#include "word_filter.h"



CompletablePageAgent::CompletablePageAgent(
    std::shared_ptr<PageDescriptionWindow> pdw,
    std::shared_ptr<IndicatorWindow> iw
)
    : page_desc_win{pdw}
    , indicator_win{iw}
    , wf{std::make_shared<word_filter>()}
    , cs{std::make_shared<CompletionStack>(*wf)}
    , ws{}
    , input_win{std::make_shared<InputWindow>(*cs, ws)}
    , completion_win{std::make_shared<CompletionWindow>(*cs, ws, *input_win, *wf)}
    , len_completion_win{std::make_shared<LengthCompletionWindow>(*cs, ws, *input_win, *wf, *completion_win)}
    , syn_win{std::make_shared<SynonymWindow>(*cs, ws, *input_win, *completion_win, *wf)}
    , ant_win{std::make_shared<AntonymWindow>(*cs, ws, *input_win, *completion_win, *syn_win, *wf)}
    , att_win{std::make_shared<AttributeWindow>(*cs, ws, *completion_win, *len_completion_win, *syn_win, *ant_win)}
    , filter_win{std::make_shared<FilterWindow>(*cs, ws, *input_win, *wf)}
    , completable_help_win{std::make_shared<CompletableHelpWindow>()}
    , completable_page{std::make_shared<CompletablePage>()}
{
    filter_win->disable(VisibilityAspect::WindowVisibility::grab());

    completable_page->add_window(page_desc_win.get());
    completable_page->add_window(indicator_win.get());
    completable_page->add_window(input_win.get());
    completable_page->add_window(completion_win.get());
    completable_page->add_window(len_completion_win.get());
    completable_page->add_window(att_win.get());
    completable_page->add_window(syn_win.get());
    completable_page->add_window(ant_win.get());
    completable_page->add_window(filter_win.get());
    completable_page->add_window(completable_help_win.get());

    completable_page->set_active_window(completion_win.get());
    completable_page->set_active_window(completable_help_win.get());

    // define neighbors for left/right arrow key switching of windows within completable page
    completion_win->set_left_neighbor(ant_win.get());
    completion_win->set_right_neighbor(syn_win.get());
    len_completion_win->set_left_neighbor(syn_win.get());
    len_completion_win->set_right_neighbor(ant_win.get());
    syn_win->set_left_neighbor(completion_win.get());
    syn_win->set_right_neighbor(len_completion_win.get());
    ant_win->set_left_neighbor(len_completion_win.get());
    ant_win->set_right_neighbor(completion_win.get());
}
