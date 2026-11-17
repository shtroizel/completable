cmake_minimum_required(VERSION 3.16.3)

project(completable VERSION 1.5.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra")
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-dangling-reference")
endif()

if(q)
    add_definitions(-DQ_ONLY)
endif()

if(EXISTS /etc/gentoo-release)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ltinfo")
endif()

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    find_program(ld_gold ld.gold)
    if(NOT "${ld_gold}" STREQUAL "ld_gold-NOTFOUND")
        message(STATUS "found gold linker!")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fuse-ld=gold")
    endif()
endif()


if(matchmaker_DL STREQUAL "ON")
    add_definitions(-DMM_DYNAMIC_LOADING)
else()
    get_filename_component(rpath_dir "${matchmaker_DIR}/.." REALPATH)
    get_filename_component(matchmaker_name "${matchmaker_DIR}/.." REALPATH)
    get_filename_component(matchmaker_name "${matchmaker_name}" NAME)
    set(
        CMAKE_INSTALL_RPATH
        "$ORIGIN/../lib/${matchmaker_name}"
        "${rpath_dir}"
    )
    find_package(matchmaker 4.0.0 REQUIRED)
endif()

find_package(Curses REQUIRED)
find_package(Threads REQUIRED)

set(
    completable_srcs
    src/AbstractCompletionDataWindow.cpp
    src/AbstractListWindow.cpp
    src/AbstractTab.cpp
    src/AbstractWindow.cpp
    src/AccessHelpWindow.cpp
    src/AntonymWindow.cpp
    src/AttributeWindow.cpp
    src/CompletableHelpWindow.cpp
    src/CompletableTab.cpp
    src/CompletableTabAgent.cpp
    src/CompletionStack.cpp
    src/CompletionWindow.cpp
    src/FilterWindow.cpp
    src/IndicatorWindow.cpp
    src/InputWindow.cpp
    src/LengthCompletionWindow.cpp
    src/MatchmakerHelpWindow.cpp
    src/MatchmakerLocationWindow.cpp
    src/MatchmakerSelectionWindow.cpp
    src/MatchmakerTab.cpp
    src/MatchmakerTabAgent.cpp
    src/OrdinalSummationWindow.cpp
    src/SettingsHelpWindow.cpp
    src/SettingsTab.cpp
    src/SettingsTabAgent.cpp
    src/SettingsWindow.cpp
    src/SynonymWindow.cpp
    src/TabDescriptionWindow.cpp
    src/completable.cpp
    src/completable_shell.cpp
    src/exec_long_task_with_busy_animation.cpp
    src/matchmaker.cpp
)

add_executable(completable ${completable_srcs})
target_link_libraries(completable Threads::Threads ${CURSES_LIBRARIES} stdc++fs)

if(matchmaker_DL STREQUAL "ON")
    target_link_libraries(completable dl)
else()
    target_link_libraries(completable matchmaker)
endif()

install(TARGETS completable DESTINATION bin)

enable_testing()

if(matchmaker_DL STREQUAL "ON" AND completable_test_library)
    set(library_args -l ${completable_test_library})
endif()
string(REPLACE ";" " " library_args_sh "${library_args}")

if(NOT matchmaker_DL STREQUAL "ON" OR completable_test_library)
    add_test(NAME shell_commands COMMAND completable ${library_args} -n -c ":len" -c "!zzzzzz" -c "zzzzzz")
    set_tests_properties(shell_commands PROPERTIES TIMEOUT 60 PASS_REGULAR_EXPRESSION "\\$ zzzzzz")

    add_test(NAME shell_no_timing COMMAND completable ${library_args} -n -c "zzzzzz")
    set_tests_properties(shell_no_timing PROPERTIES TIMEOUT 60 FAIL_REGULAR_EXPRESSION "seconds")

    add_test(NAME shell_timing_ns COMMAND completable ${library_args} -c "!zzzzzz" -c ":it 0 1" -c "zzzzzz")
    set_tests_properties(shell_timing_ns PROPERTIES TIMEOUT 60 PASS_REGULAR_EXPRESSION "nanoseconds"
        FAIL_REGULAR_EXPRESSION "microseconds")

    add_test(
        NAME shell_stdin_eof
        COMMAND sh -c "printf 'zzzzzz\\n' | \"$<TARGET_FILE:completable>\" ${library_args_sh} -n -s"
    )
    set_tests_properties(shell_stdin_eof PROPERTIES TIMEOUT 60 PASS_REGULAR_EXPRESSION "\\$ zzzzzz")

    add_test(
        NAME shell_quit
        COMMAND sh -c "printf ':q\\nzzzzzz\\n' | \"$<TARGET_FILE:completable>\" ${library_args_sh} -n -s"
    )
    set_tests_properties(shell_quit PROPERTIES TIMEOUT 60 FAIL_REGULAR_EXPRESSION "\\$ zzzzzz")

    add_test(NAME bad_option COMMAND completable --bogus)
    set_tests_properties(bad_option PROPERTIES TIMEOUT 60 WILL_FAIL TRUE)

    foreach(it :it :itl)
        add_test(NAME shell_${it}_overflow COMMAND completable ${library_args} -n -c "${it} 1 2147483647")
        set_tests_properties(
            shell_${it}_overflow PROPERTIES
            TIMEOUT 300 PASS_REGULAR_EXPRESSION "\\[ *1\\]" FAIL_REGULAR_EXPRESSION "runtime error"
        )

        add_test(NAME shell_${it}_negative COMMAND completable ${library_args} -n -c "${it} -2 3")
        set_tests_properties(
            shell_${it}_negative PROPERTIES
            TIMEOUT 60 PASS_REGULAR_EXPRESSION "\\[ *0\\]" FAIL_REGULAR_EXPRESSION "\\[ *-[0-9]"
        )
    endforeach()

    add_test(NAME shell_def_spelling COMMAND completable ${library_args} -c ":def run")
    set_tests_properties(
        shell_def_spelling PROPERTIES
        TIMEOUT 60 PASS_REGULAR_EXPRESSION "definition retrieval"
    )
endif()

find_package(Python3 COMPONENTS Interpreter)
set(curses_driver ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/curses_driver.py)

# build_and_install.py takes its long options in --name=value form (-h last, so nothing is built)
add_test(
    NAME build_script_long_values
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/build_and_install.py
        --build_dir=b --install_dir=i --matchmaker_dir=m -h
)
set_tests_properties(build_script_long_values PROPERTIES TIMEOUT 60)

if(NOT matchmaker_DL STREQUAL "ON")
    add_test(
        NAME curses_delete_from_other_window
        COMMAND ${curses_driver} $<TARGET_FILE:completable> --
            "wait:Completion \\(" text:q key:DOWN key:DOWN key:DOWN remember:4
            key:RET key:RIGHT key:DEL same:4 key:ESC
    )
    set_tests_properties(curses_delete_from_other_window PROPERTIES TIMEOUT 300 SKIP_RETURN_CODE 77)

    add_test(
        NAME curses_filter_busy_animation
        COMMAND ${curses_driver} $<TARGET_FILE:completable> --
            "wait:Completion \\(" text:q key:F1 key:F1 "wait:Completion \\(" key:ESC
    )
    set_tests_properties(curses_filter_busy_animation PROPERTIES TIMEOUT 300 SKIP_RETURN_CODE 77)

    add_test(
        NAME curses_shell_round_trip
        COMMAND ${curses_driver} $<TARGET_FILE:completable> -s --
            "wait:enter :help" text::curses key:RET "wait:Completion \\("
            text:$ "wait:enter :help" text::curses key:RET "wait:Completion \\("
            text:$ "wait:enter :help" text::q key:RET
    )
    set_tests_properties(curses_shell_round_trip PROPERTIES TIMEOUT 300 SKIP_RETURN_CODE 77)
endif()

if(matchmaker_DL STREQUAL "ON" AND completable_test_library)
    get_filename_component(test_library_dir "${completable_test_library}" DIRECTORY)
    set(libs ${CMAKE_CURRENT_BINARY_DIR}/test_library)
    add_test(
        NAME curses_book_after_failed_load
        COMMAND ${curses_driver} $<TARGET_FILE:completable> --
            "sh:rm -rf ${libs} && mkdir ${libs} && ln -s ${test_library_dir}/*.so ${libs}/"
            "wait:search location" text:${libs} key:TAB wait:libmatchmaker.so key:RET "wait:Completion \\("
            "sh:ln -sf /dev/null ${libs}/libmatchmaker.so"
            key:SHIFT_LEFT key:SHIFT_LEFT "wait:search location" key:RET
            text:$ "wait:enter :help" "text::book 0" key:RET "wait:Reading book" text::q key:RET
    )
    set_tests_properties(curses_book_after_failed_load PROPERTIES TIMEOUT 300 SKIP_RETURN_CODE 77)
endif()

if(matchmaker_DL STREQUAL "ON")
    add_library(partial_library SHARED test/partial_library.c)
    set_target_properties(
        partial_library PROPERTIES
        OUTPUT_NAME matchmaker LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/partial_library
    )
    add_test(
        NAME curses_partial_library_unloads
        COMMAND ${curses_driver} $<TARGET_FILE:completable> --
            "wait:search location" text:${CMAKE_CURRENT_BINARY_DIR}/partial_library key:TAB
            wait:libmatchmaker.so key:RET
            text:$ "wait:enter :help" "expect:*:matchmaker \\(0\\)" text::q key:RET
    )
    set_tests_properties(curses_partial_library_unloads PROPERTIES TIMEOUT 300 SKIP_RETURN_CODE 77)
endif()
