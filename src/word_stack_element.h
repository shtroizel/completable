#pragma once



class AbstractWindow;

struct word_stack_element
{
    std::string word;
    AbstractWindow * active_win;
    int display_start;
};
