cmake_minimum_required(VERSION 3.13)

project(completable VERSION 2.0.0)

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

set(
    completable_srcs
    src/AbstractPage.cpp
    src/AbstractListWindow.cpp
    src/AbstractWindow.cpp
    src/AntonymWindow.cpp
    src/AttributeWindow.cpp
    src/CompletablePage.cpp
    src/CompletionStack.cpp
    src/CompletionWindow.cpp
    src/FilterWindow.cpp
    src/HelpWindow.cpp
    src/IndicatorWindow.cpp
    src/InputWindow.cpp
    src/LengthCompletionWindow.cpp
    src/MatchmakerPage.cpp
    src/PageDescriptionWindow.cpp
    src/SettingsPage.cpp
    src/SynonymWindow.cpp
    src/completable.cpp
    src/matchmaker.cpp
)

add_executable(completable ${completable_srcs})

if(matchmaker_DL STREQUAL "ON")
    target_link_libraries(completable ${CURSES_LIBRARIES} dl)
else()
    target_link_libraries(completable matchmaker ${CURSES_LIBRARIES})
endif()

target_include_directories(
    completable
    PRIVATE
    include
    matchmaker/matchable/include
)
install(TARGETS completable DESTINATION bin)
