#include "AttributeWindow.h"

#include <cstring>
#include <iostream>

#include <ncurses.h>

#include "AntonymWindow.h"
#include "CompletionStack.h"
#include "CompletionWindow.h"
#include "Layer.h"
#include "LengthCompletionWindow.h"
#include "SynonymWindow.h"
#include "matchmaker.h"



AttributeWindow::AttributeWindow(
    CompletionStack & cs,
    WordStack & ws,
    CompletionWindow & cw,
    LengthCompletionWindow & lcw,
    SynonymWindow & sw,
    AntonymWindow & aw
)
    : AbstractCompletionDataWindow(cs, ws)
{
    cw.add_dirty_dependency(this);
    lcw.add_dirty_dependency(this);
    sw.add_dirty_dependency(this);
    aw.add_dirty_dependency(this);
}


std::string AttributeWindow::title()
{
    static std::string const t{"Attributes"};
    return t;
}


void AttributeWindow::resize_hook()
{
    height = 5;
    width = root_x;
    y = root_y - height;
    x = 0;
}


void AttributeWindow::draw_hook()
{
    auto const & c = cs.top();
    if (c.standard_completion.size() == 0)
        return;

    int selection = c.standard_completion[c.display_start];
    int line = 1;

    {
        std::string const att_label{"     Attributes:"};
        mvwprintw(w, line, 1, "%s", att_label.c_str());

        static std::string const att_name = "name";
        static std::string const att_male_name = "male name";
        static std::string const att_female_name = "female name";
        static std::string const att_place = "place";
        static std::string const att_compound = "compound";
        static std::string const att_acronym = "acronym";

        std::vector<std::string const *> attributes;
        if (matchmaker::is_name(selection))        attributes.push_back(&att_name);
        if (matchmaker::is_male_name(selection))   attributes.push_back(&att_male_name);
        if (matchmaker::is_female_name(selection)) attributes.push_back(&att_female_name);
        if (matchmaker::is_place(selection))       attributes.push_back(&att_place);
        if (matchmaker::is_compound(selection))    attributes.push_back(&att_compound);
        if (matchmaker::is_acronym(selection))     attributes.push_back(&att_acronym);

        int indent = att_label.size() + 1;
        for (auto att : attributes)
        {
            if (width - indent <= (int) att->length() + 2)
                break;

            mvwprintw(w, line, indent, "  ");
            mvwprintw(w, line, indent + 2, "%s", att->c_str());
            indent += att->length() + 2;
        }

        for (; indent < width - 1; ++indent)
            mvwaddch(w, line, indent, ' ');
    }

    ++line;

    {
        char const * const * pos{nullptr};
        int8_t const * flagged{nullptr};
        int pos_count{0};
        matchmaker::parts_of_speech(selection, &pos, &flagged, &pos_count);

        std::string const pos_label{"Parts of Speech:"};
        mvwprintw(w, line, 1, "%s", pos_label.c_str());

        int indent = pos_label.size() + 1;
        for (int pos_index = 0; pos_index < pos_count; ++pos_index)
        {
            if (!flagged[pos_index])
                continue;

            auto & p = pos[pos_index];
            int p_len = strlen(p);

            if (width - indent <= p_len + 2)
            {
                for (; indent < width - 1; ++indent)
                    mvwaddch(w, line, indent, ' ');

                ++line;
                indent = pos_label.size() + 1;
            }
            if (line > 3)
            {
                line = 3;
                break;
            }

            mvwprintw(w, line, indent, "  ");
            mvwprintw(w, line, indent + 2, "%s", p);
            indent += p_len + 2;
        }

        for (; line < 4; ++line)
        {
            for (; indent < width - 1; ++indent)
                mvwaddch(w, line, indent, ' ');

            indent = pos_label.size() + 1;
        }
    }
}


Layer::Type AttributeWindow::layer() const
{
    return Layer::Bottom::grab();
}
