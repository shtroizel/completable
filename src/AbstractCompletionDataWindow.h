#pragma once

#include "AbstractWindow.h"

#include <stack>

#include "word_stack_element.h"


class CompletionStack;
using WordStack = std::stack<word_stack_element>;

class AbstractCompletionDataWindow : public AbstractWindow
{
public:
    AbstractCompletionDataWindow(CompletionStack &, WordStack &);

protected:
    CompletionStack & cs;
    WordStack & ws;
};
