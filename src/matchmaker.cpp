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

    static int (*shim_count)(){nullptr};
    static char const * (*shim_at)(int, int *){nullptr};
    static int (*shim_lookup)(char const *, bool *){nullptr};
    static int (*shim_as_longest)(int){nullptr};
    static int (*shim_from_longest)(int){nullptr};
    static void (*shim_lengths)(int const * *, int *){nullptr};
    static bool (*shim_length_location)(int, int *, int *){nullptr};
    static int (*shim_ordinal_summation)(int){nullptr};
    static void (*shim_from_ordinal_summation)(int, int const * *, int *){nullptr};
    static bool (*shim_parts_of_speech)(int, char const * const * *, int8_t const * *, int *){nullptr};
    static bool (*shim_is_name)(int){nullptr};
    static bool (*shim_is_male_name)(int){nullptr};
    static bool (*shim_is_female_name)(int){nullptr};
    static bool (*shim_is_place)(int){nullptr};
    static bool (*shim_is_compound)(int){nullptr};
    static bool (*shim_is_acronym)(int){nullptr};
    static bool (*shim_is_phrase)(int){nullptr};
    static bool (*shim_is_used_in_book)(int, int){nullptr};
    static void (*shim_synonyms)(int, int const * *, int *){nullptr};
    static void (*shim_antonyms)(int, int const * *, int *){nullptr};
    static void (*shim_definition)(int, int const * *, int *){nullptr};
    static void (*shim_embedded)(int, int const * *, int *){nullptr};
    static void (*shim_locations)(int,
                                  int const * *,
                                  int const * *,
                                  int const * *,
                                  int const * *,
                                  int *){nullptr};
    static void (*shim_complete)(char const *, int *, int *){nullptr};
    static int (*shim_book_count)(){nullptr};
    static void (*shim_book_title)(int, int const * *, int *){nullptr};
    static void (*shim_book_author)(int, int const * *, int *){nullptr};
    static int (*shim_chapter_count)(int){nullptr};
    static void (*shim_chapter_title)(int, int, int const * *, int *){nullptr};
    static void (*shim_chapter_subtitle)(int, int, int const * *, int *){nullptr};
    static int (*shim_paragraph_count)(int, int){nullptr};
    static int (*shim_word_count)(int, int, int){nullptr};
    static int (*shim_word)(int, int, int, int, int const * *, int *, int *){nullptr};



    char * set_library(char const * so_filename)
    {
        unset_library();
        char * ret = nullptr;

#ifdef MM_DYNAMIC_LOADING
        handle = dlmopen(LM_ID_NEWLM, so_filename, RTLD_NOW);
        if (nullptr == handle)
        {
            MatchmakerState::Instance::grab().set_state(LibraryState::Unloaded::grab());
            ret = dlerror();
            return ret;
        }

        dlerror();

        bool ok = true;

        #define init_func(_f)                                                                              \
        if (ok)                                                                                            \
        {                                                                                                  \
            *(void **) (&shim_##_f) = dlsym(handle, "mm_" #_f);                                            \
            if ((ret = dlerror()) != nullptr)                                                              \
                ok = false;                                                                                \
        }
#else
        // use normal linking
        (void) so_filename; // quiet unused variable warning

        // just redirect to global namespace version of function provided by matchmaker library's header
        #define init_func(_f) shim_##_f = &::mm_##_f;
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
        init_func(is_phrase);
        init_func(is_used_in_book);
        init_func(synonyms);
        init_func(antonyms);
        init_func(definition);
        init_func(embedded);
        init_func(locations);
        init_func(complete);
        init_func(book_count);
        init_func(book_title);
        init_func(book_author);
        init_func(chapter_count);
        init_func(chapter_title);
        init_func(chapter_subtitle);
        init_func(paragraph_count);
        init_func(word_count);
        init_func(word);

#ifdef MM_DYNAMIC_LOADING
        if (ok)
            MatchmakerState::Instance::grab().set_state(LibraryState::Loaded::grab());
        else
            MatchmakerState::Instance::grab().set_state(LibraryState::Unloaded::grab());
#else
        MatchmakerState::Instance::grab().set_state(LibraryState::Linked::grab());
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

        shim_count = nullptr;
        shim_at = nullptr;
        shim_lookup = nullptr;
        shim_as_longest = nullptr;
        shim_from_longest = nullptr;
        shim_lengths = nullptr;
        shim_length_location = nullptr;
        shim_ordinal_summation = nullptr;
        shim_from_ordinal_summation = nullptr;
        shim_parts_of_speech = nullptr;
        shim_is_name = nullptr;
        shim_is_male_name = nullptr;
        shim_is_female_name = nullptr;
        shim_is_place = nullptr;
        shim_is_compound = nullptr;
        shim_is_acronym = nullptr;
        shim_is_phrase = nullptr;
        shim_is_used_in_book = nullptr;
        shim_synonyms = nullptr;
        shim_antonyms = nullptr;
        shim_definition = nullptr;
        shim_embedded = nullptr;
        shim_locations = nullptr;
        shim_complete = nullptr;
        shim_book_count = nullptr;
        shim_chapter_count = nullptr;
        shim_chapter_title = nullptr;
        shim_chapter_subtitle = nullptr;
        shim_paragraph_count = nullptr;
        shim_word_count = nullptr;
        shim_word = nullptr;
    }


    int count()
    {
        if (nullptr == shim_count)
            return 0;

        return (*shim_count)();
    }


    char const * at(int index, int * length)
    {
        static char const * empty_str = "";
        if (nullptr == shim_at)
        {
            if (nullptr != length)
                *length = 0;
            return empty_str;
        }

        return (*shim_at)(index, length);
    }


    int lookup(char const * word, bool * found)
    {
        if (nullptr == shim_lookup)
            return -1;

        return (*shim_lookup)(word, found);
    }


    int as_longest(int index)
    {
        if (nullptr == shim_as_longest)
            return -1;

        return (*shim_as_longest)(index);
    }


    int from_longest(int length_index)
    {
        if (nullptr == shim_from_longest)
            return -1;

        return (*shim_from_longest)(length_index);
    }


    void lengths(int const * * len_array, int * count)
    {
        if (nullptr == shim_lengths)
        {
            *count = 0;
            return;
        }

        (*shim_lengths)(len_array, count);
    }


    bool length_location(int length, int * length_index, int * count)
    {
        if (nullptr == shim_length_location)
        {
            *length_index = -1;
            *count = 0;
            return false;
        }

        return (*shim_length_location)(length, length_index, count);
    }


    int ordinal_summation(int index)
    {
        if (nullptr == shim_ordinal_summation)
            return 0;

        return (*shim_ordinal_summation)(index);
    }


    void from_ordinal_summation(int summation, int const * * words, int * count)
    {
        if (nullptr == shim_from_ordinal_summation)
        {
            *words = nullptr;
            *count = 0;
        }
        (*shim_from_ordinal_summation)(summation, words, count);
    }


    bool parts_of_speech(int index, char const * const * * pos, int8_t const * * flagged, int * count)
    {
        if (nullptr == shim_parts_of_speech)
        {
            *pos = nullptr;
            *flagged = nullptr;
            *count = 0;
            return false;
        }

        return (*shim_parts_of_speech)(index, pos, flagged, count);
    }


    bool is_name(int index)
    {
        if (nullptr == shim_is_name)
            return false;

        return (*shim_is_name)(index);
    }


    bool is_male_name(int index)
    {
        if (nullptr == shim_is_male_name)
            return false;

        return (*shim_is_male_name)(index);
    }


    bool is_female_name(int index)
    {
        if (nullptr == shim_is_female_name)
            return false;

        return (*shim_is_female_name)(index);
    }


    bool is_place(int index)
    {
        if (nullptr == shim_is_place)
            return false;

        return (*shim_is_place)(index);
    }


    bool is_compound(int index)
    {
        if (nullptr == shim_is_compound)
            return false;

        return (*shim_is_compound)(index);
    }


    bool is_acronym(int index)
    {
        if (nullptr == shim_is_acronym)
            return false;

        return (*shim_is_acronym)(index);
    }


    bool is_phrase(int index)
    {
        if (nullptr == shim_is_phrase)
            return false;

        return (*shim_is_phrase)(index);
    }


    bool is_used_in_book(int book_index, int index)
    {
        if (nullptr == shim_is_used_in_book)
            return false;

        return (*shim_is_used_in_book)(book_index, index);
    }


    void synonyms(int index, int const * * syn_array, int * count)
    {
        if (nullptr == shim_synonyms)
        {
            *syn_array = nullptr;
            *count = 0;
            return;
        }

        (*shim_synonyms)(index, syn_array, count);
    }


    void antonyms(int index, int const * * ant_array, int * count)
    {
        if (nullptr == shim_antonyms)
        {
            *ant_array = nullptr;
            *count = 0;
            return;
        }

        (*shim_antonyms)(index, ant_array, count);
    }


    void definition(int index, int const * * def, int * count)
    {
        if (nullptr == shim_definition)
        {
            *def = nullptr;
            *count = 0;
            return;
        }

        (*shim_definition)(index, def, count);
    }


    void embedded(int index, int const * * embedded_words, int * count)
    {
        if (nullptr == shim_embedded)
        {
            *embedded_words = nullptr;
            *count = 0;
            return;
        }

        (*shim_embedded)(index, embedded_words, count);
    }


    void locations(
        int index,
        int const * * book_indexes,
        int const * * chapter_indexes,
        int const * * paragraph_indexes,
        int const * * word_indexes,
        int * count
    )
    {
        if (nullptr == shim_locations)
        {
            *book_indexes = nullptr;
            *chapter_indexes = nullptr;
            *paragraph_indexes = nullptr;
            *word_indexes = nullptr;
            *count = 0;
            return;
        }

        (*shim_locations)(index, book_indexes, chapter_indexes, paragraph_indexes, word_indexes, count);
    }


    void complete(char const * prefix, int * start, int * length)
    {
        if (nullptr == shim_complete)
        {
            *start = -1;
            *length = 0;
            return;
        }

        (*shim_complete)(prefix, start, length);
    }


    int book_count()
    {
        if (nullptr == shim_book_count)
            return 0;

        return (*shim_book_count)();
    }


    void book_title(int book_index, int const * * title, int * count)
    {
        if (nullptr == shim_book_title)
        {
            *title = nullptr;
            *count = 0;
            return;
        }

        (*shim_book_title)(book_index, title, count);
    }


    void book_author(int book_index, int const * * author, int * count)
    {
        if (nullptr == shim_book_author)
        {
            *author = nullptr;
            *count = 0;
            return;
        }

        (*shim_book_author)(book_index, author, count);
    }


    int chapter_count(int book_index)
    {
        if (nullptr == shim_chapter_count)
            return 0;

        return (*shim_chapter_count)(book_index);
    }


    void chapter_title(int book_index, int chapter_index, int const * * title, int * count)
    {
        if (nullptr == shim_chapter_title)
        {
            *title = nullptr;
            *count = 0;
            return;
        }

        (*shim_chapter_title)(book_index, chapter_index, title, count);
    }


    void chapter_subtitle(int book_index, int chapter_index, int const * * subtitle, int * count)
    {
        if (nullptr == shim_chapter_subtitle)
        {
            *subtitle = nullptr;
            *count = 0;
            return;
        }

        (*shim_chapter_subtitle)(book_index, chapter_index, subtitle, count);
    }


    int paragraph_count(int book_index, int chapter_index)
    {
        if (nullptr == shim_paragraph_count)
            return 0;

        return (*shim_paragraph_count)(book_index, chapter_index);
    }


    int word_count(int book_index, int chapter_index, int paragraph_index)
    {
        if (nullptr == shim_word_count)
            return 0;

        return (*shim_word_count)(book_index, chapter_index, paragraph_index);
    }


    int word(
        int book_index,
        int chapter_index,
        int paragraph_index,
        int word_index,
        int const * * ancestors,
        int * ancestor_count,
        int * index_within_first_ancestor
    )
    {
        if (nullptr == shim_word)
            return -1;

        return (*shim_word)(book_index, chapter_index, paragraph_index, word_index,
                            ancestors, ancestor_count, index_within_first_ancestor);
    }
}
