cmake_minimum_required(VERSION 3.21)

project(OLKCompiler VERSION 1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(FetchContent)
enable_testing()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wpedantic -Wno-unused-function -g -Og")

# Define YYDEBUG for bison
add_compile_definitions(YYDEBUG=1)

################################################################################
#                         Flex and Bison targets                               #
################################################################################

find_package(FLEX 2.6 REQUIRED)
find_package(BISON 3.8 REQUIRED)

set(PARSER_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(LEXER_OUT "${PARSER_DIR}/joos1w_lexer.cpp")
set(PARSER_OUT "${PARSER_DIR}/joos1w_parser.cpp")

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
    COMPILE_FLAGS
    "-Wcounterexamples --report=state -v --locations --html --graph -k --color=always"
)

add_flex_bison_dependency(LEXER PARSER)

################################################################################
#                          Testing and macros                                  #
################################################################################

FetchContent_Declare(
    googletest
    URL
    https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

# Macro to add an executable with lexer and parser sources
macro(add_tool TARGET_NAME)
    add_executable(${TARGET_NAME}
        "${LEXER_OUT}"
        "${PARSER_OUT}"
        ${ARGN}
        "lib/parsetree/ParseTreePrinter.cc"
        "lib/parsetree/ParseTreeDotPrinter.cc"
    )
    target_include_directories(${TARGET_NAME} PRIVATE
        "${PARSER_DIR}"
        lib
        "."
    )
endmacro()

# Macro to add a test executable with lexer and parser sources
macro(add_my_test TARGET_NAME)
    add_tool(${TARGET_NAME} ${ARGN})
    target_link_libraries(${TARGET_NAME} GTest::gtest_main)
    gtest_discover_tests(${TARGET_NAME})
endmacro()

################################################################################
#            For every tool, create a new add_*() call below                   #
################################################################################

add_my_test(
    lexer_tests
    "tests/lexer_tester.cc"
)

add_my_test(
    parser_tests
    "tests/parser_tester.cc"
    "tools/gen_fragments/FragmentGenerator.cc"
)

add_tool(
    gen_fragments
    "tools/gen_fragments/main.cc"
    "tools/gen_fragments/FragmentGenerator.cc"
)

add_tool(
    scanner
    "tools/scanner/main.cc"
)

add_tool(
    parser
    "tools/parser/main.cc"
)

add_tool(
    AST
    "lib/ast/main.cc"
    "lib/parsetree/ParseTreeVisitor.cc"
)
