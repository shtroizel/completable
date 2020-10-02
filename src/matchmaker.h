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


#include <string>
#include <vector>


namespace matchmaker
{
    char * set_library(char const * so_filename);
    void unset_library();

    // matchmaker interface
    int size();
    std::string const & at(int index);
    int lookup(std::string const & word, bool * found);
    int as_longest(int index);
    int from_longest(int length_index);
    std::vector<std::size_t> const & lengths();
    bool length_location(std::size_t length, int & length_index, int & count);
    std::vector<std::string> const & all_parts_of_speech();
    std::vector<int8_t> const & flagged_parts_of_speech(int index);
    void parts_of_speech(int index, std::vector<std::string const *> & pos);
    bool is_name(int index);
    bool is_male_name(int index);
    bool is_female_name(int index);
    bool is_place(int index);
    bool is_compound(int index);
    bool is_acronym(int index);
    std::vector<int> const & synonyms(int index);
    std::vector<int> const & antonyms(int index);
    void complete(std::string const & prefix, int & start, int & length);
}
