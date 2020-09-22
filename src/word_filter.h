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


#include <functional>

#include <matchable/matchable.h>

#include <matchmaker/matchmaker.h>


using filter_func = std::function<bool (int)>;

PROPERTYx1_MATCHABLE(
    filter_func,
    func,
    word_attribute,
    name,
    male_spc_name,
    female_spc_name,
    place,
    compound,
    acronym,
    all_spc_labels_spc_missing  // must be last entry! see all_labels_missing()
)


static bool all_labels_missing(int word)
{
    for (int i = 0; i < (int) word_attribute::variants().size() - 1; ++i)
        if (word_attribute::from_index(i).as_func()(word))
            return false;

    return true;
}



SET_PROPERTY(word_attribute, name, func, &matchmaker::is_name);
SET_PROPERTY(word_attribute, male_spc_name, func, &matchmaker::is_male_name);
SET_PROPERTY(word_attribute, female_spc_name, func, &matchmaker::is_female_name);
SET_PROPERTY(word_attribute, place, func, &matchmaker::is_place);
SET_PROPERTY(word_attribute, compound, func, &matchmaker::is_compound);
SET_PROPERTY(word_attribute, acronym, func, &matchmaker::is_acronym);
SET_PROPERTY(word_attribute, all_spc_labels_spc_missing, func, &all_labels_missing);

MATCHABLE(filter_direction, exclusive, inclusive)

struct word_filter
{
    word_attribute::Flags attributes;
    filter_direction::Type direction{filter_direction::exclusive::grab()};

    bool passes(int word) const
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

        return false;
    }
};
