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
            for (int i = 1; i < matchmaker::size(); i *= 10)
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
                      << "matchmaker (" << matchmaker::size() << ") $  ";
        else
            std::cout << "matchmaker (" << matchmaker::size() << ") { enter :help for help } $  ";

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
            matchmaker::complete(line.substr(1), completion_start, completion_length);
            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
            std::cout << "completion (" << completion_length << ") : ";
            for (int i = completion_start; i < completion_start + completion_length; ++i)
                std::cout << " " << matchmaker::at(i);
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
                for (auto l : matchmaker::lengths())
                {
                    if (matchmaker::length_location(l, index, count))
                        std::cout << "    " << std::setw(MAX_INDEX_DIGITS) << l
                                  << " letter words begin at index [" << std::setw(MAX_INDEX_DIGITS)
                                  << index << "] with a count of: " << std::to_string(count) << std::endl;
                    else
                        std::cout << "OOPS!" << std::endl; // should be impossible
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
                index = matchmaker::lookup(words[0], &found);
                auto stop = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                if (index < matchmaker::size())
                {
                    std::cout << index << "], length[" << matchmaker::as_longest(index) << "] :  '"
                              << matchmaker::at(index) << "' ";
                    if (!found)
                        std::cout << "(index if existed) ";
                }
                else
                {
                    std::cout << matchmaker::size() << "], (would be new last word)";
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
                index = matchmaker::lookup(words[1], &found);
                if (index == matchmaker::size())
                {
                    std::cout << matchmaker::size() << "], (would be new last word)" << std::endl;
                    continue;
                }
                auto synonyms = matchmaker::synonyms(index);
                auto stop = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                std::cout << index << "] :  '" << matchmaker::at(index) << "' ";
                if (!found)
                    std::cout << "(index if existed) ";
                std::cout << " syn:  ";
                for (int j = 0; j < (int) synonyms.size() - 1; ++j)
                    std::cout << matchmaker::at(synonyms[j]) << ", ";
                if (synonyms.size() > 0)
                    std::cout << matchmaker::at(synonyms[synonyms.size() - 1]);
                else
                    std::cout << " NONE AVAILABLE";
                std::cout << "\n       -------> lookup + synonym retrieval time: "
                          << duration.count() << " microseconds" << std::endl;
            }
            else if (words[0] == ":a")
            {
                std::cout << "       [";
                auto start = std::chrono::high_resolution_clock::now();
                index = matchmaker::lookup(words[1], &found);
                if (index == matchmaker::size())
                {
                    std::cout << matchmaker::size() << "], (would be new last word)" << std::endl;
                    continue;
                }
                auto antonyms = matchmaker::antonyms(index);
                auto stop = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                std::cout << index << "] :  '" << matchmaker::at(index) << "' ";
                if (!found)
                    std::cout << "(index if existed) ";
                std::cout << " ant:  ";
                for (int j = 0; j < (int) antonyms.size() - 1; ++j)
                    std::cout << matchmaker::at(antonyms[j]) << ", ";
                if (antonyms.size() > 0)
                    std::cout << matchmaker::at(antonyms[antonyms.size() - 1]);
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
                    index = matchmaker::lookup(words[i], &found);
                    if (index == matchmaker::size())
                    {
                        std::cout << matchmaker::size() << "], (would be new last word)" << std::endl;
                        continue;
                    }
                    auto start = std::chrono::high_resolution_clock::now();
                    matchmaker::parts_of_speech(index, pos);
                    auto stop = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                    std::cout << index << "] :  '" << matchmaker::at(index) << "' ";
                    if (!found)
                        std::cout << "(index if existed) ";
                    std::cout << " pos:  ";
                    for (int j = 0; j < (int) pos.size() - 1; ++j)
                        std::cout << *pos[j] << ", ";

                    if (pos.size() > 0)
                        std::cout << *pos[pos.size() - 1];
                    else
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
                for (int i = start; i < (int) matchmaker::size() && i < start + count; ++i)
                    std::cout << "       [" << std::setw(MAX_INDEX_DIGITS) << matchmaker::from_longest(i)
                              << "], length[" << std::setw(MAX_INDEX_DIGITS) << i << "]  "
                              << matchmaker::at(matchmaker::from_longest(i)) << " has "
                              << matchmaker::at(matchmaker::from_longest(i)).length()
                              << " characters" << std::endl;
            }
            else if (words[0] == ":it")
            {
                for (int i = start; i < matchmaker::size() && i < start + count; ++i)
                {
                    auto start = std::chrono::high_resolution_clock::now();
                    std::string const & str = matchmaker::at(i);
                    auto stop = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                    std::cout << "       [" << std::setw(MAX_INDEX_DIGITS) << i << "], length["
                              << std::setw(MAX_INDEX_DIGITS) << matchmaker::as_longest(i) << "] :  '"
                              << str << "' accessed in " << duration.count() << " microseconds\n";
                }
                std::cout << std::flush;
            }
        }
    }
}
