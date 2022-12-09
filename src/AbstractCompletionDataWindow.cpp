#include "AbstractCompletionDataWindow.h"

#include "CompletionStack.h"



AbstractCompletionDataWindow::AbstractCompletionDataWindow(
    CompletionStack & completion_stack,
    WordStack & word_stack
)
    : AbstractWindow()
    , cs{completion_stack}, ws{word_stack}
{
}
