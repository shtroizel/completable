#pragma once



class AbstractListWindow;

struct word_stack_element
{
    std::string word;
    AbstractListWindow * active_win;
    int display_start;
};
