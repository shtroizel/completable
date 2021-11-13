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


#include "matchmaker.h"
#include "MatchmakerState.h"

#include <iostream>

#ifdef MM_DYNAMIC_LOADING
    #include <dlfcn.h>
#else
    #ifdef Q_ONLY
        #include <matchmaker_q/matchmaker.h>
    #else
        #include <matchmaker/matchmaker.h>
    #endif
#endif



namespace matchmaker
{
#ifdef MM_DYNAMIC_LOADING
    static void * handle{nullptr};
#endif

    static int (*mm_count)(){nullptr};
    static char const * (*mm_at)(int, int *){nullptr};
    static int (*mm_lookup)(char const *, bool *){nullptr};
    static int (*mm_as_longest)(int){nullptr};
    static int (*mm_from_longest)(int){nullptr};
    static void (*mm_lengths)(int const * *, int *){nullptr};
    static bool (*mm_length_location)(int, int *, int *){nullptr};
    static int (*mm_ordinal_summation)(int){nullptr};
    static void (*mm_from_ordinal_summation)(int, int const * *, int *){nullptr};
    static bool (*mm_parts_of_speech)(int, char const * const * *, int8_t const * *, int *){nullptr};
    static bool (*mm_is_name)(int){nullptr};
    static bool (*mm_is_male_name)(int){nullptr};
    static bool (*mm_is_female_name)(int){nullptr};
    static bool (*mm_is_place)(int){nullptr};
    static bool (*mm_is_compound)(int){nullptr};
    static bool (*mm_is_acronym)(int){nullptr};
    static void (*mm_synonyms)(int, int const * *, int *){nullptr};
    static void (*mm_antonyms)(int, int const * *, int *){nullptr};
    static void (*mm_complete)(char const *, int *, int *){nullptr};



    char * set_library(char const * so_filename)
    {
        unset_library();
        char * ret = nullptr;

#ifdef MM_DYNAMIC_LOADING
        handle = dlmopen(LM_ID_NEWLM, so_filename, RTLD_NOW);
        if (nullptr == handle)
        {
            MatchmakerStateInstance::Instance::grab().set_state(MatchmakerState::Unloaded::grab());
            ret = dlerror();
            return ret;
        }

        dlerror();

        bool ok = true;

        #define init_func(_f)                                                                              \
        if (ok)                                                                                            \
        {                                                                                                  \
            *(void **) (&mm_##_f) = dlsym(handle, "mm_" #_f);                                              \
            if ((ret = dlerror()) != nullptr)                                                              \
                ok = false;                                                                                \
        }
#else
        // use normal linking
        (void) so_filename; // quiet unused variable warning

        // just redirect to global namespace version of function provided by matchmaker library's header
        #define init_func(_f) mm_##_f = &::mm_##_f;
#endif

        init_func(count);
        init_func(at);
        init_func(lookup);
        init_func(as_longest);
        init_func(from_longest);
        init_func(lengths);
        init_func(length_location);
        init_func(ordinal_summation);
        init_func(from_ordinal_summation);
        init_func(parts_of_speech);
        init_func(is_name);
        init_func(is_male_name);
        init_func(is_female_name);
        init_func(is_place);
        init_func(is_compound);
        init_func(is_acronym);
        init_func(synonyms);
        init_func(antonyms);
        init_func(complete);

#ifdef MM_DYNAMIC_LOADING
        if (ok)
            MatchmakerStateInstance::Instance::grab().set_state(MatchmakerState::Loaded::grab());
        else
            MatchmakerStateInstance::Instance::grab().set_state(MatchmakerState::Unloaded::grab());
#else
        MatchmakerStateInstance::Instance::grab().set_state(MatchmakerState::Linked::grab());
#endif

        return ret;
    }


    void unset_library()
    {
#ifdef MM_DYNAMIC_LOADING
        if (nullptr != handle)
        {
            dlclose(handle);
            handle = nullptr;
        }
#endif

        mm_count = nullptr;
        mm_at = nullptr;
        mm_lookup = nullptr;
        mm_as_longest = nullptr;
        mm_from_longest = nullptr;
        mm_lengths = nullptr;
        mm_length_location = nullptr;
        mm_ordinal_summation = nullptr;
        mm_from_ordinal_summation = nullptr;
        mm_parts_of_speech = nullptr;
        mm_is_name = nullptr;
        mm_is_male_name = nullptr;
        mm_is_female_name = nullptr;
        mm_is_place = nullptr;
        mm_is_compound = nullptr;
        mm_is_acronym = nullptr;
        mm_synonyms = nullptr;
        mm_antonyms = nullptr;
        mm_complete = nullptr;
    }


    int count()
    {
        if (nullptr == mm_count)
            return 0;

        return (*mm_count)();
    }


    char const * at(int index, int * length)
    {
        static char const * empty_str = "";
        if (nullptr == mm_at)
        {
            if (nullptr != length)
                *length = 0;
            return empty_str;
        }

        return (*mm_at)(index, length);
    }


    int lookup(char const * word, bool * found)
    {
        if (nullptr == mm_lookup)
            return -1;

        return (*mm_lookup)(word, found);
    }


    int as_longest(int index)
    {
        if (nullptr == mm_as_longest)
            return -1;

        return (*mm_as_longest)(index);
    }


    int from_longest(int length_index)
    {
        if (nullptr == mm_from_longest)
            return -1;

        return (*mm_from_longest)(length_index);
    }


    void lengths(int const * * len_array, int * count)
    {
        if (nullptr == mm_lengths)
        {
            *count = 0;
            return;
        }

        (*mm_lengths)(len_array, count);
    }


    bool length_location(int length, int * length_index, int * count)
    {
        if (nullptr == mm_length_location)
        {
            *length_index = -1;
            *count = 0;
            return false;
        }

        return (*mm_length_location)(length, length_index, count);
    }


    int ordinal_summation(int index)
    {
        if (nullptr == mm_ordinal_summation)
            return 0;

        return (*mm_ordinal_summation)(index);
    }


    void from_ordinal_summation(int summation, int const * * words, int * count)
    {
        if (nullptr == mm_from_ordinal_summation)
        {
            *words = nullptr;
            *count = 0;
        }
        (*mm_from_ordinal_summation)(summation, words, count);
    }


    bool parts_of_speech(int index, char const * const * * pos, int8_t const * * flagged, int * count)
    {
        if (nullptr == mm_parts_of_speech)
        {
            *pos = nullptr;
            *flagged = nullptr;
            *count = 0;
            return false;
        }

        return (*mm_parts_of_speech)(index, pos, flagged, count);
    }


    bool is_name(int index)
    {
        if (nullptr == mm_is_name)
            return false;

        return (*mm_is_name)(index);
    }


    bool is_male_name(int index)
    {
        if (nullptr == mm_is_male_name)
            return false;

        return (*mm_is_male_name)(index);
    }


    bool is_female_name(int index)
    {
        if (nullptr == mm_is_female_name)
            return false;

        return (*mm_is_female_name)(index);
    }


    bool is_place(int index)
    {
        if (nullptr == mm_is_place)
            return false;

        return (*mm_is_place)(index);
    }


    bool is_compound(int index)
    {
        if (nullptr == mm_is_compound)
            return false;

        return (*mm_is_compound)(index);
    }


    bool is_acronym(int index)
    {
        if (nullptr == mm_is_acronym)
            return false;

        return (*mm_is_acronym)(index);
    }


    void synonyms(int index, int const * * syn_array, int * count)
    {
        if (nullptr == mm_synonyms)
        {
            *syn_array = nullptr;
            *count = 0;
            return;
        }

        (*mm_synonyms)(index, syn_array, count);
    }


    void antonyms(int index, int const * * ant_array, int * count)
    {
        if (nullptr == mm_antonyms)
        {
            *ant_array = nullptr;
            *count = 0;
            return;
        }

        (*mm_antonyms)(index, ant_array, count);
    }


    void complete(char const * prefix, int * start, int * length)
    {
        if (nullptr == mm_complete)
        {
            *start = -1;
            *length = 0;
            return;
        }

        (*mm_complete)(prefix, start, length);
    }
}
