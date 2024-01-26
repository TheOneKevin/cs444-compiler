cmake_minimum_required(VERSION 3.21)

project(OLKCompiler VERSION 1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)

find_package(FLEX 2.6 REQUIRED)
find_package(BISON 3.8 REQUIRED)

set(PARSER_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(LEXER_OUT "${PARSER_DIR}/joos1w_lexer.c")
set(PARSER_OUT "${PARSER_DIR}/joos1w_parser.c")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

flex_target(
    LEXER
    "lib/grammar/joos1w_lexer.l"
    "${LEXER_OUT}"
    DEFINES_FILE
    "${PARSER_DIR}/lexer.h"
)
bison_target(
    PARSER
    "lib/grammar/joos1w_parser.y"
    "${PARSER_OUT}"
    DEFINES_FILE
    "${PARSER_DIR}/parser.h"
)
add_flex_bison_dependency(LEXER PARSER)

################################################################################
#           For every tool, create a new add_executable() call below           #
################################################################################

message(STATUS "${LEXER_OUT}")

add_executable(
    helloworld
    "${LEXER_OUT}"
    "${PARSER_OUT}"
    "tools/helloworld/main.cc"
)
target_include_directories(helloworld PRIVATE "${PARSER_DIR}")
