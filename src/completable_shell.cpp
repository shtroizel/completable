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
                      << "{ use  :it <index> <count>        to iterate <count> words from <index>       }\n"
                      << "{ use  :pos <word>                for parts of speech of <word>               }\n"
                      << "{ use  :s <word>                  for synonyms of <word>                      }\n"
                      << "{ use  :a <word>                  for antonyms of <word>                      }\n"
                      << "{ use  :itl <index> <count>       like ':it' but uses length indexes          }\n"
                      << "{ use  :len                       to list length index offsets                }\n"
                      << "{ use  :help                      to toggle help                              }\n"
                      << "matchmaker (" << matchmaker::count() << ") $  ";
        else
            std::cout << "matchmaker (" << matchmaker::count() << ") { enter :help for help } $  ";

        std::string line;
        std::getline(std::cin, line);
        if (std::cin.eof() == 1)
        {
            std::cout << std::endl;
            break;
        }
        std::vector<std::string> words;
        std::stringstream ss(line);
        std::string token;
        while (std::getline(ss, token, ' '))
            words.push_back(token);

        if (words.size() == 0)
        {
            std::cout << std::endl;
            break;
        }
        else if (words[0].length() && words[0][0] == '!')
        {
            int completion_start{0};
            int completion_length{0};
            auto start = std::chrono::high_resolution_clock::now();
            matchmaker::complete(line.substr(1).c_str(), &completion_start, &completion_length);
            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
            std::cout << "completion (" << completion_length << ") : ";
            for (int i = completion_start; i < completion_start + completion_length; ++i)
                std::cout << " " << matchmaker::at(i, nullptr);
            std::cout << "\ncompletion done in " << duration.count() << " microseconds" << std::endl;
        }
        else if (words.size() == 1 && words[0].size() > 0)
        {
            if (words[0] == ":q")
            {
                exit(EXIT_SUCCESS);
            }
            else if (words[0] == ":len")
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
                                  << " letter words begin at index [" << std::setw(MAX_INDEX_DIGITS)
                                  << index << "] with a count of: " << std::to_string(count) << std::endl;
                    else
                        std::cout << "index [" << l << "] out of bounds! expected range: [0.."
                                  << matchmaker::count() << "]" << std::endl;
                }
            }
            else if (words[0] == ":help")
            {
                help = !help;
            }
            else
            {
                std::cout << "       [";
                auto start = std::chrono::high_resolution_clock::now();
                index = matchmaker::lookup(words[0].c_str(), &found);
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
        else if (words.size() == 2)
        {
            if (words[0] == ":s")
            {
                std::cout << "       [";
                auto start = std::chrono::high_resolution_clock::now();
                index = matchmaker::lookup(words[1].c_str(), &found);
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
            else if (words[0] == ":a")
            {
                std::cout << "       [";
                auto start = std::chrono::high_resolution_clock::now();
                index = matchmaker::lookup(words[1].c_str(), &found);
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
            else if (words[0] == ":pos")
            {
                for (int i = 1; i < (int) words.size(); ++i)
                {
                    std::cout << "       [";
                    index = matchmaker::lookup(words[i].c_str(), &found);
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
            }
        }
        else if (words.size() == 3)
        {
            int start{0}; try { start = std::stoi(words[1]); } catch (...) { continue; }
            int count{0}; try { count = std::stoi(words[2]); } catch (...) { continue; }

            if (words[0] == ":itl")
            {
                for (int i = start; i < (int) matchmaker::count() && i < start + count; ++i)
                {
                    int word_len{0};
                    char const * word = matchmaker::at(matchmaker::from_longest(i), &word_len);
                    std::cout << "       [" << std::setw(MAX_INDEX_DIGITS) << matchmaker::from_longest(i)
                              << "], length[" << std::setw(MAX_INDEX_DIGITS) << i << "]  "
                              << word << " has " << word_len << " characters" << std::endl;
                }
            }
            else if (words[0] == ":it")
            {
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
        }
    }
}
