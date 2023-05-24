cmake_minimum_required(VERSION 3.17)

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
    get_filename_component(matchmaker_name "${matchmaker_DIR}/.." REALPATH)
    get_filename_component(matchmaker_name "${matchmaker_name}" NAME)
    set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib/${matchmaker_name}")
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
    src/TabDescriptionWindow.cpp
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
