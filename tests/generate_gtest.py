import os, shutil

# Get the directory of this file
dir_path = os.path.dirname(os.path.realpath(__file__))
gen_path = os.path.join(dir_path, "generated")

# Does gen_path exist? If so, remove it
if os.path.exists(gen_path):
    shutil.rmtree(gen_path)
os.makedirs(gen_path)

# List all the files in the directory
a1_files = os.listdir(os.path.join(dir_path, "data", "a1"))

################################################################################

template = """
#include <iostream>
#include <gtest/gtest.h>
#include "../common.h"
"""

# For each file, add a test case for the parser rules
a1_parser_tests = ['r7', 'r9', 'r10', 'r11', 'r14', 'r15', 'r16']
for file in a1_files:
    if not file.endswith('.java'):
        continue
    if file.split('_')[0] not in a1_parser_tests:
        continue
    EXPECT_WHAT = 'EXPECT_TRUE'
    if file.split('_')[1] == 'invalid':
        EXPECT_WHAT = 'EXPECT_FALSE'
    with open(os.path.join(dir_path, "data", "a1", file), 'r') as f:
        content = f.read()
    test_name = file.split('.')[0]
    # Replace \ with \\ and \n with \\n and " with \"
    content = content.replace('\\', '\\\\').replace('\n', '\\n').replace('"', '\\"')
    template += f"""
        TEST(ParserTests, {test_name}) {{
            std::string input = "{content}";
            {EXPECT_WHAT}(testing::parse_grammar(input));
        }}
    """

# Output the template to parser_a1_tests.cpp
with open(os.path.join(gen_path, "parser_a1_tests.cc"), 'w') as f:
    f.write(template)

################################################################################

template = """
#include <iostream>
#include <gtest/gtest.h>
#include "../common.h"
"""

for file in a1_files:
    rule = file.split('_')[0]
    if not file.endswith('.java'):
        continue
    EXPECT_WHAT = 'EXPECT_TRUE'
    if file.split('_')[1] == 'invalid':
        EXPECT_WHAT = 'EXPECT_FALSE'
    if rule == 'r1' or rule == 'r8':
        EXPECT_WHAT = 'EXPECT_TRUE'
    with open(os.path.join(dir_path, "data", "a1", file), 'r') as f:
        content = f.read()
    test_name = file.split('.')[0]
    # Replace \ with \\ and \n with \\n and " with \"
    content = content.replace('\\', '\\\\').replace('\n', '\\n').replace('"', '\\"')
    template += f"""
        TEST(AstTests, {test_name}) {{
            std::string input = "{content}";
            {EXPECT_WHAT}(testing::build_ast(input));
        }}
    """
with open(os.path.join(gen_path, "ast_a1_tests.cc"), 'w') as f:
    f.write(template)
