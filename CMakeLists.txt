cmake_minimum_required(VERSION 3.13)

project(completable VERSION 1.0.2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra")

# work around broken ncurses
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


get_filename_component(rpath_dir "${matchmaker_DIR}/.." REALPATH)
set(CMAKE_INSTALL_RPATH "${rpath_dir}")

find_package(matchmaker 1.4 REQUIRED)

find_package(Curses REQUIRED)

set(
    completable_srcs
    src/AbstractWindow.cpp
    src/CompletionStack.cpp
    src/CompletionWindow.cpp
    src/InputWindow.cpp
    src/completable.cpp
)

add_executable(completable ${completable_srcs})
target_link_libraries(completable matchmaker ${CURSES_LIBRARIES})
target_include_directories(completable PRIVATE include)
install(TARGETS completable DESTINATION bin)
