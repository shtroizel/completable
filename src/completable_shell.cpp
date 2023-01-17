#include "completable_shell.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "matchmaker.h"



void completable_shell()
{
    int index{-1};
    bool found{false};
    bool help{false};
    std::vector<std::string const *> pos;
    int const MAX_INDEX_DIGITS =
        []()
        {
            int digit_count{0};
            for (int i = 1; i < matchmaker::count(); i *= 10)
                ++digit_count;

            return digit_count;
        }();

    while (true)
    {
        std::cout << "\n\n";
        if (help)
            std::cout << "{ just enter a word for lookup                                                }\n"
                      << "{ prefix the word with '!' for completion                                     }\n"
                      << "{ use  :it <index> <count>        to iterate <count> terms from <index>       }\n"
                      << "{ use  :pos <word>                for parts of speech of <word>               }\n"
                      << "{ use  :s <word>                  for synonyms of <word>                      }\n"
                      << "{ use  :a <word>                  for antonyms of <word>                      }\n"
                      << "{ use  :itl <index> <count>       like ':it' but uses length indexes          }\n"
                      << "{ use  :len                       to list length index offsets                }\n"
                      << "{ use  :e <index>                 to list embedded terms                      }\n"
                      << "{ use  :books                     to list books                               }\n"
                      << "{ use  :book <index>              to read a book                              }\n"
                      << "{ use  :loc <index>               to locate all occurrences of a word         }\n"
                      << "{ use  :p <b> <ch> <p> <w>        show a word's parent and index within parent}\n"
                      << "{ use  :curses                    return to curses mode                       }\n"
                      << "{ use  :q                         to quit                                     }\n"
                      << "{ use  :help                      to toggle help                              }\n"
                      << "matchmaker (" << matchmaker::count() << ") $  ";
        else
            std::cout << "matchmaker (" << matchmaker::count() << ") { enter :help for help } $  ";

        std::string line;
        std::getline(std::cin, line);
        if (std::cin.fail() || std::cin.eof() == 1)
        {
            std::cin.clear();
            std::cin.ignore();
            std::cout << "\ncommand failed! terminal resized?" << std::endl;
            std::getline(std::cin, line);
            continue;
        }
        std::vector<std::string> terms;
        std::stringstream ss(line);
        std::string token;
        while (std::getline(ss, token, ' '))
            terms.push_back(token);

        if (terms.size() == 0 || terms[0].length() == 0)
        {
            std::cout << std::endl;
            continue;
        }
        if (terms[0][0] == '!')
        {
            int completion_start{0};
            int completion_length{0};
            auto start = std::chrono::high_resolution_clock::now();
            matchmaker::complete(line.substr(1).c_str(), &completion_start, &completion_length);
            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
            std::cout << "completion (" << completion_length << ") :\n";
            for (int i = completion_start; i < completion_start + completion_length; ++i)
                std::cout << "    --> " << matchmaker::at(i, nullptr) << "\n";
            std::cout << "\ncompletion done in " << duration.count() << " microseconds" << std::endl;
        }
        else if (terms[0] == ":it")
        {
            if (terms.size() < 3)
                continue;

            int start{0}; try { start = std::stoi(terms[1]); } catch (...) { continue; }
            int count{0}; try { count = std::stoi(terms[2]); } catch (...) { continue; }

            for (int i = start; i < matchmaker::count() && i < start + count; ++i)
            {
                auto start = std::chrono::high_resolution_clock::now();
                char const * word = matchmaker::at(i, nullptr);
                auto stop = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                std::cout << "       [" << std::setw(MAX_INDEX_DIGITS) << i << "], length["
                            << std::setw(MAX_INDEX_DIGITS) << matchmaker::as_longest(i) << "] :  '"
                            << word << "' accessed in " << duration.count() << " microseconds\n";
            }
            std::cout << std::flush;
        }
        else if (terms[0] == ":pos")
        {
            if (terms.size() < 2)
                continue;

            std::cout << "       [";
            index = matchmaker::lookup(line.substr(5).c_str(), &found);
            if (index == matchmaker::count())
            {
                std::cout << matchmaker::count() << "], (would be new last word)" << std::endl;
                continue;
            }
            auto start = std::chrono::high_resolution_clock::now();
            char const * const * pos{nullptr};
            int8_t const * flagged{nullptr};
            int pos_count{0};
            matchmaker::parts_of_speech(index, &pos, &flagged, &pos_count);
            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
            std::cout << index << "] :  '" << matchmaker::at(index, nullptr) << "' ";
            if (!found)
                std::cout << "(index if existed) ";
            std::cout << " pos (" << pos_count << "):  ";
            for (int j = 0; j < pos_count; ++j)
                if (flagged[j])
                    std::cout << pos[j] << ", ";

            if (pos_count == 0)
                std::cout << "NONE AVAILABLE";

            std::cout << "\n       -------> parts_of_speech() time: "
                        << duration.count() << " microseconds" << std::endl;
        }
        else if (terms[0] == ":s")
        {
            if (terms.size() < 2)
                continue;

            std::cout << "       [";
            auto start = std::chrono::high_resolution_clock::now();
            index = matchmaker::lookup(line.substr(3).c_str(), &found);
            if (index == matchmaker::count())
            {
                std::cout << matchmaker::count() << "], (would be new last word)" << std::endl;
                continue;
            }
            int const * syn_array{nullptr};
            int syn_array_count{0};
            matchmaker::synonyms(index, &syn_array, &syn_array_count);
            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
            std::cout << index << "] :  '" << matchmaker::at(index, nullptr) << "' ";
            if (!found)
                std::cout << "(index if existed) ";
            std::cout << " syn:  ";
            for (int j = 0; j < syn_array_count - 1; ++j)
                std::cout << matchmaker::at(syn_array[j], nullptr) << ", ";
            if (syn_array_count > 0)
                std::cout << matchmaker::at(syn_array[syn_array_count - 1], nullptr);
            else
                std::cout << " NONE AVAILABLE";
            std::cout << "\n       -------> lookup + synonym retrieval time: "
                        << duration.count() << " microseconds" << std::endl;
        }
        else if (terms[0] == ":a")
        {
            if (terms.size() < 2)
                continue;

            std::cout << "       [";
            auto start = std::chrono::high_resolution_clock::now();
            index = matchmaker::lookup(line.substr(3).c_str(), &found);
            if (index == matchmaker::count())
            {
                std::cout << matchmaker::count() << "], (would be new last word)" << std::endl;
                continue;
            }
            int const * ant_array{nullptr};
            int ant_array_count{0};
            matchmaker::antonyms(index, &ant_array, &ant_array_count);
            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
            std::cout << index << "] :  '" << matchmaker::at(index, nullptr) << "' ";
            if (!found)
                std::cout << "(index if existed) ";
            std::cout << " ant:  ";
            for (int j = 0; j < ant_array_count - 1; ++j)
                std::cout << matchmaker::at(ant_array[j], nullptr) << ", ";
            if (ant_array_count > 0)
                std::cout << matchmaker::at(ant_array[ant_array_count - 1], nullptr);
            else
                std::cout << " NONE AVAILABLE";
            std::cout << "\n       -------> lookup + antonym retrieval time: "
                        << duration.count() << " microseconds" << std::endl;
        }
        else if (terms[0] == ":def")
        {
            if (terms.size() < 2)
                continue;

            std::cout << "       [";
            auto start = std::chrono::high_resolution_clock::now();
            index = matchmaker::lookup(line.substr(5).c_str(), &found);
            if (index == matchmaker::count())
            {
                std::cout << matchmaker::count() << "], (would be new last word)" << std::endl;
                continue;
            }
            int const * def{nullptr};
            int def_count{0};
            matchmaker::definition(index, &def, &def_count);
            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
            std::cout << index << "] :  '" << matchmaker::at(index, nullptr) << "' ";
            if (!found)
                std::cout << "(index if existed) ";
            std::cout << " def: ";
            for (int j = 0; j < def_count; ++j)
                std::cout << " " << matchmaker::at(def[j], nullptr);
            if (def_count == 0)
                std::cout << " NONE AVAILABLE";
            std::cout << "\n       -------> lookup + definiton retrieval time: "
                        << duration.count() << " microseconds" << std::endl;
        }
        else if (terms[0] == ":itl")
        {
            if (terms.size() < 3)
                continue;

            int start{0}; try { start = std::stoi(terms[1]); } catch (...) { continue; }
            int count{0}; try { count = std::stoi(terms[2]); } catch (...) { continue; }

            for (int i = start; i < (int) matchmaker::count() && i < start + count; ++i)
            {
                int word_len{0};
                char const * word = matchmaker::at(matchmaker::from_longest(i), &word_len);
                std::cout << "       [" << std::setw(MAX_INDEX_DIGITS) << matchmaker::from_longest(i)
                            << "], length[" << std::setw(MAX_INDEX_DIGITS) << i << "]  "
                            << word << " has " << word_len << " characters" << std::endl;
            }
        }
        else if (terms[0] == ":len")
        {
            std::cout << "The following length indexes can be used with ':itl'" << std::endl;

            int index{0};
            int count{0};

            int const * lengths_array{nullptr};
            int lengths_array_count{0};
            matchmaker::lengths(&lengths_array, &lengths_array_count);

            for (int i = 0; i < lengths_array_count; ++i)
            {
                auto & l = lengths_array[i];

                if (matchmaker::length_location(l, &index, &count))
                    std::cout << "    " << std::setw(MAX_INDEX_DIGITS) << l
                                << " letter terms begin at index [" << std::setw(MAX_INDEX_DIGITS)
                                << index << "] with a count of: " << std::to_string(count) << std::endl;
                else
                    std::cout << "index [" << l << "] out of bounds! expected range: [0.."
                                << matchmaker::count() << "]" << std::endl;
            }
        }
        else if (terms[0] == ":e")
        {
            if (terms.size() < 2)
                continue;

            int index{0}; try { index = std::stoi(line.substr(3)); } catch (...) { continue; }

            int const * embedded_terms;
            int count{0};
            matchmaker::embedded(index, &embedded_terms, &count);
            char const * compound_word = matchmaker::at(index, nullptr);

            std::cout << "       [" << std::setw(MAX_INDEX_DIGITS) << index << "], length["
                        << std::setw(MAX_INDEX_DIGITS) << matchmaker::as_longest(index) << "] :  '"
                        << compound_word << "'\n   *** has the following embedded terms: ***\n";
            for (int i = 0; i < count; ++i)
                std::cout << "       --> [" << std::setw(MAX_INDEX_DIGITS) << embedded_terms[i] << "], length["
                            << std::setw(MAX_INDEX_DIGITS) << matchmaker::as_longest(embedded_terms[i]) << "] :  '"
                            << matchmaker::at(embedded_terms[i], nullptr) << "'\n";
            std::cout << std::flush;
        }
        else if (terms[0] == ":books")
        {
            std::cout << "The number of books in the library is: " << matchmaker::book_count()
                      << std::endl << std::endl;
            for (int i = 0; i < matchmaker::book_count(); ++i)
            {
                std::cout << "    [" << std::to_string(i) << "]\n"
                            << "           title: ";
                int const * title{nullptr};
                int title_count{0};
                matchmaker::book_title(i, &title, &title_count);
                for (int t = 0; t < title_count; ++t)
                    std::cout << matchmaker::at(title[t], nullptr);

                std::cout << "\n          author: ";
                int const * author{nullptr};
                int author_count{0};
                matchmaker::book_author(i, &author, &author_count);
                for (int t = 0; t < author_count; ++t)
                    std::cout << matchmaker::at(author[t], nullptr);

                std::cout << "\n        chapters: "
                            << std::to_string(matchmaker::chapter_count(i)) << "\n" << std::endl;
            }
        }
        else if (terms[0] == ":book")
        {
            if (terms.size() < 2)
                continue;

            int book_index{0}; try { book_index = std::stoi(line.substr(6)); } catch (...) { continue; }

            std::cout << "Reading book [" << terms[1] << "]...\n"
                      << "\n               title: ";
            int const * title{nullptr};
            int title_count{0};
            matchmaker::book_title(book_index, &title, &title_count);
            for (int t = 0; t < title_count; ++t)
                std::cout << matchmaker::at(title[t], nullptr);

            std::cout << "\n              author: ";
            int const * author{nullptr};
            int author_count{0};
            matchmaker::book_author(book_index, &author, &author_count);
            for (int t = 0; t < author_count; ++t)
                std::cout << matchmaker::at(author[t], nullptr);

            std::cout << "\n            chapters: "
                        << std::to_string(matchmaker::chapter_count(book_index)) << "\n" << std::endl;

            for (int ch = 0; ch < matchmaker::chapter_count(book_index); ++ch)
            {
                std::cout << "\n***********************************************************************\n";
                std::cout << "   chapter title: ";
                int const * ch_title{nullptr};
                int ch_title_count{0};
                matchmaker::chapter_title(book_index, ch, &ch_title, &ch_title_count);
                for (int t = 0; t < ch_title_count; ++t)
                    std::cout << matchmaker::at(ch_title[t], nullptr);

                std::cout << "\nchapter subtitle: ";
                int const * ch_subtitle{nullptr};
                int ch_subtitle_count{0};
                matchmaker::chapter_subtitle(book_index, ch, &ch_subtitle, &ch_subtitle_count);
                for (int t = 0; t < ch_subtitle_count; ++t)
                    std::cout << matchmaker::at(ch_subtitle[t], nullptr);
                std::cout << "\n-----------------------------------------------------------------------"
                          << std::endl;
                for (int p = 0; p < matchmaker::paragraph_count(book_index, ch); ++p)
                {
                    for (int w = 0; w < matchmaker::word_count(book_index, ch, p); ++w)
                    {
                        int ancestor_count = 0;
                        int const * ancestors;
                        int index_within_first_ancestor{-1};
                        int term = matchmaker::word(book_index,
                                                    ch,
                                                    p,
                                                    w,
                                                    &ancestors,
                                                    &ancestor_count,
                                                    &index_within_first_ancestor,
                                                    nullptr);

                        std::cout << " " << matchmaker::at(term, nullptr);
                    }
                    std::cout << std::endl;
                }
                std::cout << "***********************************************************************\n\n\n"
                          << std::endl;
            }
        }
        else if (terms[0] == ":p")
        {
            if (terms.size() != 5)
                continue;

            int bk{0}; try { bk = std::stoi(terms[1]); } catch (...) { continue; }
            int ch{0}; try { ch = std::stoi(terms[2]); } catch (...) { continue; }
            int par{0}; try { par = std::stoi(terms[3]); } catch (...) { continue; }
            int wrd{0}; try { wrd = std::stoi(terms[4]); } catch (...) { continue; }

            int ancestor_count{0};
            int const * ancestors{nullptr};
            int index_within_first_ancestor{-1};
            int term = matchmaker::word(
                           bk,
                           ch,
                           par,
                           wrd,
                           &ancestors,
                           &ancestor_count,
                           &index_within_first_ancestor,
                           nullptr
                       );

            std::cout << "\n"
                      << "\n         given term: " << term
                      << "\n        term as str: " << matchmaker::at(term, nullptr)
                      << "\n          ancestors:";

            for (int i = 0; i < ancestor_count; ++i)
                std::cout << " " << ancestors[i];

            std::cout << "\n    ancestors (str): ";

            for (int i = 0; i < ancestor_count; ++i)
                std::cout << " " << matchmaker::at(ancestors[i], nullptr);

            std::cout << "\nindex within parent: " << index_within_first_ancestor << std::endl;
        }
        else if (terms[0] == ":loc")
        {
            if (terms.size() < 2)
                continue;

            int search_word{0}; try { search_word = std::stoi(line.substr(5)); } catch (...) { continue; }

            int const * book_indexes{nullptr};
            int const * chapter_indexes{nullptr};
            int const * paragraph_indexes{nullptr};
            int const * word_indexes{nullptr};
            int count{0};
            matchmaker::locations(search_word,
                                  &book_indexes,
                                  &chapter_indexes,
                                  &paragraph_indexes,
                                  &word_indexes,
                                  &count);
            std::cout << matchmaker::at(search_word, nullptr) << "\n";
            for (int i = 0; i < count; ++i)
            {
                std::cout << "  ----> ("
                          << std::to_string(book_indexes[i]) << ", "
                          << std::to_string(chapter_indexes[i]) << ", "
                          << std::to_string(paragraph_indexes[i]) << ", "
                          << std::to_string(word_indexes[i]) << ")" << std::endl;
            }
            if (count == 0)
                std::cout << "  ----> NONE!" << std::endl;
        }
        else if (terms[0] == ":curses")
        {
            break;
        }
        else if (terms[0] == ":q")
        {
            exit(EXIT_SUCCESS);
        }
        else if (terms[0] == ":help")
        {
            help = !help;
        }
        else
        {
            std::cout << "       [";
            auto start = std::chrono::high_resolution_clock::now();
            index = matchmaker::lookup(line.c_str(), &found);
            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
            if (index < matchmaker::count())
            {
                std::cout << index << "], length[" << matchmaker::as_longest(index) << "] :  '"
                            << matchmaker::at(index, nullptr) << "' ";
                if (!found)
                    std::cout << "(index if existed) ";
            }
            else
            {
                std::cout << matchmaker::count() << "], (would be new last word)";
            }
            std::cout << "       lookup time: " << duration.count() << " microseconds" << std::endl;
        }
    }
}
