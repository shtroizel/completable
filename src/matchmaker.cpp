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


#ifdef MM_DL
    #include <dlfcn.h>
#else
    #include <matchmaker/matchmaker.h>
#endif



namespace matchmaker
{
#ifdef MM_DL
    static void * handle{nullptr};
#endif

    static int (*mm_size)(){nullptr};
    static std::string const & (*mm_at)(int){nullptr};
    static int (*mm_lookup)(std::string const &, bool *){nullptr};
    static int (*mm_as_longest)(int){nullptr};
    static int (*mm_from_longest)(int){nullptr};
    static std::vector<std::size_t> const & (*mm_lengths)(){nullptr};
    static bool (*mm_length_location)(std::size_t, int &, int &){nullptr};
    static std::vector<std::string> const & (*mm_all_parts_of_speech)(){nullptr};
    static std::vector<int8_t> const & (*mm_flagged_parts_of_speech)(int){nullptr};
    static void (*mm_parts_of_speech)(int, std::vector<std::string const *> &){nullptr};
    static bool (*mm_is_name)(int){nullptr};
    static bool (*mm_is_male_name)(int){nullptr};
    static bool (*mm_is_female_name)(int){nullptr};
    static bool (*mm_is_place)(int){nullptr};
    static bool (*mm_is_compound)(int){nullptr};
    static bool (*mm_is_acronym)(int){nullptr};
    static std::vector<int> const & (*mm_synonyms)(int){nullptr};
    static std::vector<int> const & (*mm_antonyms)(int){nullptr};
    static void (*mm_complete)(std::string const &, int &, int &){nullptr};



    char * set_library(char const * so_filename)
    {
        unset_library();
        char * ret = nullptr;

#ifdef MM_DL
        handle = dlopen(so_filename, RTLD_NOW);
        if (nullptr == handle)
        {
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
        (void) so_filename;
        #define init_func(_f) mm_##_f = &::mm_##_f;
#endif

        init_func(size);
        init_func(at);
        init_func(lookup);
        init_func(as_longest);
        init_func(from_longest);
        init_func(lengths);
        init_func(length_location);
        init_func(all_parts_of_speech);
        init_func(flagged_parts_of_speech);
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

        return ret;
    }


    void unset_library()
    {
#ifdef MM_DL
        if (nullptr != handle)
        {
            dlclose(handle);
            handle = nullptr;
        }
#endif

        mm_size = nullptr;
        mm_at = nullptr;
        mm_lookup = nullptr;
        mm_as_longest = nullptr;
        mm_from_longest = nullptr;
        mm_lengths = nullptr;
        mm_length_location = nullptr;
        mm_all_parts_of_speech = nullptr;
        mm_flagged_parts_of_speech = nullptr;
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


    int size()
    {
        if (nullptr == mm_size)
            return 0;

        return (*mm_size)();
    }


    std::string const & at(int index)
    {
        static std::string const empty_str;
        if (nullptr == mm_at)
            return empty_str;

        return (*mm_at)(index);
    }


    int lookup(std::string const & word, bool * found)
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


    std::vector<std::size_t> const & lengths()
    {
        static std::vector<std::size_t> const empty_vect;
        if (nullptr == mm_lengths)
            return empty_vect;

        return (*mm_lengths)();
    }


    bool length_location(std::size_t length, int & index, int & count)
    {
        if (nullptr == mm_length_location)
            return false;

        return (*mm_length_location)(length, index, count);
    }


    std::vector<std::string> const & all_parts_of_speech()
    {
        static std::vector<std::string> const empty_vect;
        if (nullptr == mm_all_parts_of_speech)
            return empty_vect;

        return (*mm_all_parts_of_speech)();
    }


    std::vector<int8_t> const & flagged_parts_of_speech(int index)
    {
        static std::vector<int8_t> const empty_vect;
        if (nullptr == mm_flagged_parts_of_speech)
            return empty_vect;

        return (*mm_flagged_parts_of_speech)(index);
    }


    void parts_of_speech(int index, std::vector<std::string const *> & pos)
    {
        if (nullptr == mm_parts_of_speech)
            return;

        (*mm_parts_of_speech)(index, pos);
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


    std::vector<int> const & synonyms(int index)
    {
        static std::vector<int> const empty_vect;
        if (nullptr == mm_synonyms)
            return empty_vect;

        return (*mm_synonyms)(index);
    }


    std::vector<int> const & antonyms(int index)
    {
        static std::vector<int> const empty_vect;
        if (nullptr == mm_antonyms)
            return empty_vect;

        return (*mm_antonyms)(index);
    }


    void complete(std::string const & prefix, int & start, int & length)
    {
        if (nullptr == mm_complete)
            return;

        (*mm_complete)(prefix, start, length);
    }
}
