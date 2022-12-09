#pragma once

#include <functional>

#include <matchable/matchable.h>

#include "matchmaker.h"


using filter_func = std::function<bool (int)>;

PROPERTYx1_MATCHABLE(
    // property
    filter_func, func,

    // matchable name
    word_attribute,

    // variants
    name,
    male_spc_name,
    female_spc_name,
    place,
    compound,
    acronym,
    phrase,
    used_spc_in_spc_Crumbs,
    all_spc_labels_spc_missing  // must be last entry! see all_labels_missing()
)


static bool all_labels_missing(int word)
{
    for (int i = 0; i < (int) word_attribute::variants().size() - 1; ++i)
        if (word_attribute::from_index(i).as_func()(word))
            return false;

    return true;
}


static bool used_in_Crumbs(int term)
{
    return matchmaker::is_used_in_book(0, term);
}


MATCHABLE_VARIANT_PROPERTY_VALUE(word_attribute, name, func, &matchmaker::is_name);
MATCHABLE_VARIANT_PROPERTY_VALUE(word_attribute, male_spc_name, func, &matchmaker::is_male_name);
MATCHABLE_VARIANT_PROPERTY_VALUE(word_attribute, female_spc_name, func, &matchmaker::is_female_name);
MATCHABLE_VARIANT_PROPERTY_VALUE(word_attribute, place, func, &matchmaker::is_place);
MATCHABLE_VARIANT_PROPERTY_VALUE(word_attribute, compound, func, &matchmaker::is_compound);
MATCHABLE_VARIANT_PROPERTY_VALUE(word_attribute, acronym, func, &matchmaker::is_acronym);
MATCHABLE_VARIANT_PROPERTY_VALUE(word_attribute, phrase, func, &matchmaker::is_phrase);
MATCHABLE_VARIANT_PROPERTY_VALUE(word_attribute, used_spc_in_spc_Crumbs, func, &used_in_Crumbs);
MATCHABLE_VARIANT_PROPERTY_VALUE(word_attribute, all_spc_labels_spc_missing, func, &all_labels_missing);

MATCHABLE(filter_direction, exclusive, inclusive)
MATCHABLE(filter_logic, or_logic, and_logic)

struct word_filter
{
    word_attribute::Flags attributes;
    filter_direction::Type direction{filter_direction::exclusive::grab()};
    filter_logic::Type logic{filter_logic::and_logic::grab()};

    bool passes(int word) const
    {
        if (logic == filter_logic::or_logic::grab())
        {
            if (direction == filter_direction::exclusive::grab())
            {
                for (auto att : attributes.currently_set())
                    if (att.as_func()(word))
                        return false;

                return true;
            }
            else if (direction == filter_direction::inclusive::grab())
            {
                for (auto att : attributes.currently_set())
                    if (att.as_func()(word))
                        return true;

                return false;
            }
        }
        else if (logic == filter_logic::and_logic::grab())
        {
            if (direction == filter_direction::exclusive::grab())
            {
                for (auto att : attributes.currently_set())
                    if (!att.as_func()(word))
                        return true;

                return 0 == attributes.currently_set().size();
            }
            else if (direction == filter_direction::inclusive::grab())
            {
                for (auto att : attributes.currently_set())
                    if (!att.as_func()(word))
                        return false;

                return true;
            }
        }

        return false;
    }
};
