#pragma once

#include <string>
#include <vector>

/**
 * completable_shell() implements a simple, non-ncurses command-line app using the matchmaker library
 *
 * Given commands, it runs them in order and returns instead of reading stdin.
 */
void completable_shell(std::vector<std::string> const & commands, bool show_timing);
