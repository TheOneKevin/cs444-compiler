cmake_minimum_required(VERSION 3.21)

project(OLKCompiler VERSION 1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(FetchContent)
enable_testing()

# Option to always produce colored output
option (FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." TRUE)
if (${FORCE_COLORED_OUTPUT})
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
       add_compile_options (-fdiagnostics-color=always)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
       add_compile_options (-fcolor-diagnostics)
    endif()
endif()

# Set debug mode flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Wpedantic -Wno-unused-function -Wno-unused-private-field -g -gdwarf-4 -O0")

# Set release mode flags
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -g")

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
    "${PARSER_DIR}/joos1w.lexer.do.not.use.h"
)

bison_target(
    PARSER
    "lib/grammar/joos1w_parser.y"
    "${PARSER_OUT}"
    DEFINES_FILE
    "${PARSER_DIR}/joos1w.parser.tab.h"
    COMPILE_FLAGS
    "--report=state -v --locations --html --graph -k --color=always"
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

# Create a library target for our compiler with sources under lib/
add_library(OLKCompilerLib
    # flex bison
    "${LEXER_OUT}"
    "${PARSER_OUT}"
    # ast
    "lib/ast/Decl.cc"
    "lib/ast/DeclContext.cc"
    "lib/ast/Expr.cc"
    "lib/ast/Stmt.cc"
    "lib/ast/Modifiers.cc"
    # semantic
    "lib/semantic/Semantic.cc"
    "lib/semantic/NameResolver.cc"
    "lib/semantic/HierarchyChecker.cc"
    "lib/semantic/TypeChecker.cc"
    # grammar
    "lib/grammar/Joos1W.cc"
    # parsetree
    "lib/parsetree/ParseTree.cc"
    "lib/parsetree/ParseTreeVisitor.cc"
    "lib/parsetree/VisitClassInterface.cc"
    "lib/parsetree/VisitExpression.cc"
    "lib/parsetree/VisitLeaf.cc"
    "lib/parsetree/VisitStatement.cc"
    # utils
    "lib/utils/DotPrinter.cc"
    "lib/utils/PassManager.cc"
    # Passes
    "lib/passes/CompilerPasses.cc"
)
target_include_directories(
    OLKCompilerLib
    PUBLIC
    "lib"
    "${PARSER_DIR}"
)

# Macro to add an executable with lexer and parser sources
macro(add_tool TARGET_NAME)
    add_executable(${TARGET_NAME}
        ${ARGN}
    )
    target_link_libraries(${TARGET_NAME} OLKCompilerLib)
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
#            Run python program right now to generate the tests                #
################################################################################

execute_process(
    COMMAND python3 "scripts/generate_gtest.py"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

file(GLOB GENERATED_TEST_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/tests/generated/*.cc)

################################################################################
#              For every tool, create a new add_*() call below                 #
################################################################################

add_my_test(
    all_tests
    "tests/common.cc"
    "tests/lexer_tests.cc"
    "tests/parser_tests.cc"
    "tests/parser_more_tests.cc"
    "tests/ast_tests.cc"
    "tools/genfrags/basic_fragments.cc"
    "tools/genfrags/class_fragments.cc"
    "tools/genfrags/method_fragments.cc"
    "tools/genfrags/FragmentGenerator.cc"
    ${GENERATED_TEST_SRCS}
)

add_tool(
    genfrags
    "tools/genfrags/main.cc"
    "tools/genfrags/basic_fragments.cc"
    "tools/genfrags/class_fragments.cc"
    "tools/genfrags/method_fragments.cc"
    "tools/genfrags/FragmentGenerator.cc"
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
    joosc
    "tools/joosc/main.cc"
)

add_tool(
    jcc1
    "tools/jcc1/main.cc"
)
