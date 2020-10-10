cmake_minimum_required(VERSION 3.13)

project(completable VERSION 1.2.4)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra")

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
    add_definitions(-DMM_DL)
else()
    get_filename_component(rpath_dir "${matchmaker_DIR}/.." REALPATH)
    set(CMAKE_INSTALL_RPATH "${rpath_dir}")
    find_package(matchmaker 2.0.0 REQUIRED)
endif()

find_package(Curses REQUIRED)
find_package(Threads REQUIRED)

set(
    completable_srcs
    src/AbstractCompletionDataWindow.cpp
    src/AbstractPage.cpp
    src/AbstractListWindow.cpp
    src/AbstractWindow.cpp
    src/AntonymWindow.cpp
    src/AttributeWindow.cpp
    src/CompletableHelpWindow.cpp
    src/CompletablePage.cpp
    src/CompletablePageAgent.cpp
    src/CompletionStack.cpp
    src/CompletionWindow.cpp
    src/FilterWindow.cpp
    src/IndicatorWindow.cpp
    src/InputWindow.cpp
    src/LengthCompletionWindow.cpp
    src/MatchmakerPage.cpp
    src/PageDescriptionWindow.cpp
    src/SettingsHelpMessageWindow.cpp
    src/SettingsHelpWindow.cpp
    src/SettingsPage.cpp
    src/SettingsPageAgent.cpp
    src/SettingsWindow.cpp
    src/SynonymWindow.cpp
    src/completable.cpp
    src/completable_shell.cpp
    src/matchmaker.cpp
)

add_executable(completable ${completable_srcs})
target_link_libraries(completable Threads::Threads ${CURSES_LIBRARIES})

if(matchmaker_DL STREQUAL "ON")
    target_link_libraries(completable dl)
else()
    target_link_libraries(completable matchmaker)
endif()

target_include_directories(
    completable
    PRIVATE
    include
    matchable/include
)
install(TARGETS completable DESTINATION bin)
