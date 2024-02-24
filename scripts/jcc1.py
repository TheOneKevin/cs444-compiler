#!/usr/bin/env python3

import os
import subprocess
import sys

# Grab the assignment number from the command line
if len(sys.argv) != 3:
    print(f"Usage: python3 {sys.argv[0]} <assignment> <test>")
    print("Where <test> is the name of the .java file **OR** the directory")
    print(f"Example: python3 {sys.argv[0]} a2 J2_hierachyCheck25")
    sys.exit(1)


# Recursively grab all java files subroutine
def grab_all_java(dir):
    java_files = []
    for root, _, files in os.walk(dir):
        for file in files:
            if file.endswith(".java"):
                java_files.append(os.path.join(root, file))
    return java_files


# Get the directory of this script
script_dir = os.path.dirname(os.path.realpath(__file__))
assignment = sys.argv[1]
test = sys.argv[2]
assignment_number = assignment[1:]

# Set up the test directories
test = f"/u/cs444/pub/assignment_testcases/{assignment}/{test}"
stdlib_dir = f"/u/cs444/pub/stdlib/{assignment_number}.0"
joosc = os.path.join(script_dir, "..", "build", "jcc1")

# Check that both the stdlib directory exist and is a directory
if not os.path.exists(stdlib_dir) or not os.path.isdir(stdlib_dir):
    print(f"Error: stdlib directory {stdlib_dir} does not exist")
    sys.exit(1)

# Check that the test directory exist (or is a file)
if not os.path.exists(test):
    print(f"Error: test directory {test} does not exist")
    sys.exit(1)

# List test cases files
stdlib_files = grab_all_java(stdlib_dir)

files = [test] if os.path.isfile(test) else grab_all_java(test)
files += stdlib_files
ret = subprocess.run([joosc, *files], stderr=subprocess.PIPE, stdout=subprocess.PIPE)
print("return code:")
print(ret.returncode)
print("stdout:")
print(ret.stdout.decode("utf-8"))
print("stderr:")
print(ret.stderr.decode("utf-8"))
sys.exit(ret.returncode)
