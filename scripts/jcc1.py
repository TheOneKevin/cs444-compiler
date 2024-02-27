#!/usr/bin/env python3

import argparse
import os
import subprocess
import sys

parser = argparse.ArgumentParser(description="Run the joosc compiler on a test case")
parser.add_argument("assignment", help="The assignment number (a1, a2, etc)")
parser.add_argument("test", help="The test case to run")
parser.add_argument("args", nargs="*", help="Additional arguments to pass to joosc")
parser.epilog = (
    "Where <test> is the name of the .java file **OR** the directory"
    "Example: python3 {sys.argv[0]} a2 J2_hierachyCheck25"
)
args = parser.parse_args()

# If args is not set, set it to [-c]
if not args.args:
    args.args = ["-c"]

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
assignment = args.assignment
test = args.test
assignment_number = max(2, int(assignment[1:]))

# Set up the test directories
test = f"/u/cs444/pub/assignment_testcases/{assignment}/{test}"
stdlib_dir = f"/u/cs444/pub/stdlib/{assignment_number}.0"
joosc = os.path.join(script_dir, "..", "build", "jcc1")
# Grab the joosc binary from the environment
if "JOOSC" in os.environ:
    joosc = os.environ["JOOSC"]

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
arglist = [joosc, *args.args, *files]
ret = subprocess.run(arglist, stderr=subprocess.PIPE, stdout=subprocess.PIPE)

print("Command:")
print(subprocess.list2cmdline(arglist))
print("return code:")
print(ret.returncode)
print("stdout:")
print(ret.stdout.decode("utf-8", errors="ignore"))
print("stderr:")
print(ret.stderr.decode("utf-8", errors="ignore"))
sys.exit(ret.returncode)
