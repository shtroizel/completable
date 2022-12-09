#pragma once

#include "AbstractCompletionDataWindow.h"



class CompletionWindow;
class LengthCompletionWindow;
class SynonymWindow;
class AntonymWindow;


/**
 * The AttributeWindow provides attributes including parts of speech for the currently completed word
 */
class AttributeWindow : public AbstractCompletionDataWindow
{
public:
    AttributeWindow(
        CompletionStack &,
        WordStack &,
        CompletionWindow &,
        LengthCompletionWindow &,
        SynonymWindow &,
        AntonymWindow &
    );

private:
    std::string title() final;
    void resize_hook() final;
    void draw_hook() final;
    Layer::Type layer() const final;
};
