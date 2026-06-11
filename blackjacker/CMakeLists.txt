cmake_minimum_required(VERSION 3.20)

project(
    BlackJacker
    VERSION 0.1.0
    DESCRIPTION "Blackjack counting practice and game simulator"
    LANGUAGES C
)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

find_package(Curses REQUIRED)
find_package(SQLite3 REQUIRED)
find_program(CLANG_FORMAT_EXE NAMES clang-format)

set(CLANG_FORMAT_STYLE
    "file:${CMAKE_SOURCE_DIR}/configuration/.clang-format"
)

find_path(YAML_INCLUDE_DIR
    NAMES yaml.h
    PATHS
        /opt/homebrew/include
        /opt/homebrew/opt/libyaml/include
        /usr/local/include
        /usr/local/opt/libyaml/include
)

find_library(YAML_LIBRARY
    NAMES yaml
    PATHS
        /opt/homebrew/lib
        /opt/homebrew/opt/libyaml/lib
        /usr/local/lib
        /usr/local/opt/libyaml/lib
)

if(NOT YAML_INCLUDE_DIR OR NOT YAML_LIBRARY)
    message(FATAL_ERROR "libyaml is required. Install it with your package manager, e.g. `brew install libyaml`.")
endif()

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/source/*.c"
)

file(GLOB_RECURSE PROJECT_FORMAT_FILES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/source/*.c"
    "${CMAKE_SOURCE_DIR}/source/*.h"
)

add_executable(blackjacker ${PROJECT_SOURCES})

target_include_directories(blackjacker
    PRIVATE
        "${CMAKE_SOURCE_DIR}/source"
        "${CURSES_INCLUDE_DIRS}"
        "${YAML_INCLUDE_DIR}"
        "${SQLite3_INCLUDE_DIRS}"
)

target_link_libraries(blackjacker
    PRIVATE
        "${CURSES_LIBRARIES}"
        "${YAML_LIBRARY}"
        SQLite::SQLite3
)

if(MSVC)
    target_compile_options(blackjacker PRIVATE /W4)
else()
    target_compile_options(blackjacker PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(CLANG_FORMAT_EXE)
    add_custom_target(format
        COMMAND "${CLANG_FORMAT_EXE}" "--style=${CLANG_FORMAT_STYLE}" -i ${PROJECT_FORMAT_FILES}
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Formatting source files"
        VERBATIM
    )
endif()
