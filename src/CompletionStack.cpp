#include "CompletionStack.h"

#include <queue>

#include "MatchmakerState.h"
#include "matchmaker.h"
#include "word_filter.h"



CompletionStack::CompletionStack(word_filter const & f) : wf(f)
{
    clear_top();
}


void CompletionStack::push(int ch)
{
    if (MatchmakerState::Instance::grab().as_state() == LibraryState::Unloaded::grab())
        return;

    if (completion_count >= CAPACITY)
        return;

    {
        // keep reference to previous top for prefix initialization
        completion const & prev_top = top();

        // grow
        ++completion_count;

        clear_top();
        top().prefix = prev_top.prefix;
    }
    top().prefix += ch;

    // get new completion
    {
        int start{0};
        int length{0};
        matchmaker::complete(
            top().prefix.c_str(),
            &start,
            &length
        );
        top().standard_completion.reserve(length);
        for (int i = start; i < start + length; ++i)
            if (wf.passes(i))
                top().standard_completion.push_back(i);

        // if adding 'ch' would make an unknown word then ignore (undo) the push
        if (top().standard_completion.size() == 0)
        {
            pop();
            return;
        }
    }

    // calculate length completion
    top().length_completion.reserve(top().standard_completion.size());
    std::make_heap(top().length_completion.begin(), top().length_completion.end());
    for (auto const & i : top().standard_completion)
    {
        top().length_completion.push_back(matchmaker::as_longest(i));
        std::push_heap(top().length_completion.begin(), top().length_completion.end());
    }
    std::sort_heap(top().length_completion.begin(), top().length_completion.end());
    top().display_start = 0;
    top().len_display_start = 0;
}


void CompletionStack::pop()
{
    if (completion_count > 1)
        --completion_count;
}


void CompletionStack::clear_top()
{
    top().prefix.clear();
    top().standard_completion.clear();
    top().display_start = 0;
    top().len_display_start = 0;
    top().length_completion.clear();
    top().ord_sum_display_start = 0;
    top().syn_display_start = 0;
    top().ant_display_start = 0;

    if (completion_count == 1)
    {
        // use entire dictionary for completions

        top().standard_completion.reserve(matchmaker::count());
        for (int i = 0; i < matchmaker::count(); ++i)
            if (wf.passes(i))
                top().standard_completion.push_back(i);

        top().length_completion.reserve(top().standard_completion.size());
        std::make_heap(top().length_completion.begin(), top().length_completion.end());
        for (auto const & i : top().standard_completion)
        {
            top().length_completion.push_back(matchmaker::as_longest(i));
            std::push_heap(top().length_completion.begin(), top().length_completion.end());
        }
        std::sort_heap(top().length_completion.begin(), top().length_completion.end());
    }
}


void CompletionStack::clear_all()
{
    for (completion_count = 2; completion_count <= CAPACITY; ++completion_count)
        clear_top();

    completion_count = 1;
    clear_top();
}
