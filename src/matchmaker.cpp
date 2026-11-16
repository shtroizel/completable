#include "matchmaker.h"
#include "MatchmakerState.h"

#include <iostream>
#include <string>

#ifdef MM_DYNAMIC_LOADING
    #include <dlfcn.h>
#else
    #ifdef Q_ONLY
        #include <matchmaker_q/matchmaker.h>
    #else
        #include <matchmaker/matchmaker.h>
    #endif
#endif



// every matchmaker function completable calls: name, return type, parameter types
#define MM_FUNCTIONS(X)                                                                          \
    X(count, int, ())                                                                            \
    X(at, char const *, (int, int *))                                                            \
    X(lookup, int, (char const *, bool *))                                                       \
    X(as_longest, int, (int))                                                                    \
    X(from_longest, int, (int))                                                                  \
    X(lengths, void, (int const * *, int *))                                                     \
    X(length_location, bool, (int, int *, int *))                                                \
    X(ordinal_summation, int, (int))                                                             \
    X(from_ordinal_summation, void, (int, int const * *, int *))                                 \
    X(parts_of_speech, bool, (int, char const * const * *, int8_t const * *, int *))             \
    X(is_name, bool, (int))                                                                      \
    X(is_male_name, bool, (int))                                                                 \
    X(is_female_name, bool, (int))                                                               \
    X(is_place, bool, (int))                                                                     \
    X(is_compound, bool, (int))                                                                  \
    X(is_acronym, bool, (int))                                                                   \
    X(is_phrase, bool, (int))                                                                    \
    X(is_used_in_book, bool, (int, int))                                                         \
    X(synonyms, void, (int, int const * *, int *))                                               \
    X(antonyms, void, (int, int const * *, int *))                                               \
    X(definition, void, (int, int const * *, int *))                                             \
    X(embedded, void, (int, int const * *, int *))                                               \
    X(locations, void, (int, int const * *, int const * *, int const * *, int const * *, int *)) \
    X(complete, void, (char const *, int *, int *))                                              \
    X(book_count, int, ())                                                                       \
    X(book_title, void, (int, int const * *, int *))                                             \
    X(book_author, void, (int, int const * *, int *))                                            \
    X(chapter_count, int, (int))                                                                 \
    X(chapter_title, void, (int, int, int const * *, int *))                                     \
    X(chapter_subtitle, void, (int, int, int const * *, int *))                                  \
    X(paragraph_count, int, (int, int))                                                          \
    X(word_count, int, (int, int, int))                                                          \
    X(word, int, (int, int, int, int, int const * *, int *, int *, bool *))



namespace matchmaker
{
#ifdef MM_DYNAMIC_LOADING
    static void * handle{nullptr};
#endif

    #define DECLARE_SHIM(name, ret, params) static ret (*shim_##name) params{nullptr};
    MM_FUNCTIONS(DECLARE_SHIM)
    #undef DECLARE_SHIM



    char * load_library(char const * so_filename)
    {
        unset_library();

#ifdef MM_DYNAMIC_LOADING
        // copied before anything else can call into libdl and replace dlerror()'s text
        static std::string error;

        handle = dlmopen(LM_ID_NEWLM, so_filename, RTLD_NOW);
        if (nullptr == handle)
        {
            error = dlerror();
            return error.data();
        }

        dlerror();

        char const * missing{nullptr};
        #define LOAD_SHIM(name, ret, params)                                                               \
            if (nullptr == missing)                                                                        \
            {                                                                                              \
                *(void **) (&shim_##name) = dlsym(handle, "mm_" #name);                                    \
                missing = dlerror();                                                                       \
            }
        MM_FUNCTIONS(LOAD_SHIM)
        #undef LOAD_SHIM

        if (nullptr != missing)
        {
            error = missing;
            unset_library();
            return error.data();
        }
#else
        (void) so_filename;

        // the linked library's own functions; their types must match MM_FUNCTIONS
        #define LINK_SHIM(name, ret, params) shim_##name = &::mm_##name;
        MM_FUNCTIONS(LINK_SHIM)
        #undef LINK_SHIM
#endif

        return nullptr;
    }


    void publish_library_state()
    {
#ifdef MM_DYNAMIC_LOADING
        if (nullptr != handle)
            MatchmakerState::Instance::grab().set_state(LibraryState::Loaded::grab());
        else
            MatchmakerState::Instance::grab().set_state(LibraryState::Unloaded::grab());
#else
        MatchmakerState::Instance::grab().set_state(LibraryState::Linked::grab());
#endif
    }


    char * set_library(char const * so_filename)
    {
        char * ret = load_library(so_filename);
        publish_library_state();
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

        #define CLEAR_SHIM(name, ret, params) shim_##name = nullptr;
        MM_FUNCTIONS(CLEAR_SHIM)
        #undef CLEAR_SHIM
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
            return;
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
        int * index_within_first_ancestor,
        bool * referenced
    )
    {
        if (nullptr == shim_word)
            return -1;

        return (*shim_word)(book_index, chapter_index, paragraph_index, word_index,
                            ancestors, ancestor_count, index_within_first_ancestor, referenced);
    }
}
