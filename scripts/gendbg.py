#!/usr/bin/env python3

import json
import os
import sys
import argparse


def build_json(name, args):
    # Grab JOOSC from the environment
    path = "${workspaceFolder}/build/jcc1"
    if "JOOSC" in os.environ:
        path = os.environ["JOOSC"]
    return json.dumps(
        {
            "version": "0.2.0",
            "configurations": [
                {
                    "type": "lldb",
                    "request": "launch",
                    "name": name,
                    "program": path,
                    "args": args,
                    "cwd": "${workspaceFolder}",
                }
            ],
        }, indent=4
    )


# Recursively grab all java files subroutine
def grab_all_java(dir):
    java_files = []
    for root, _, files in os.walk(dir):
        for file in files:
            if file.endswith(".java"):
                java_files.append(os.path.join(root, file))
    return java_files

# Parse the arguments
parser = argparse.ArgumentParser(description="Generate a launch.json file for debugging jcc1")
parser.add_argument("assignment", help="The assignment number (a1, a2, etc)")
parser.add_argument("test", help="The test case to run")
parser.add_argument("args", nargs="*", help="Additional arguments to pass to jcc1")
parser.epilog = (
    "Where <test> is the name of the .java file **OR** the directory"
    "Example: python3 {sys.argv[0]} a2 J2_hierachyCheck25"
)
args = parser.parse_args()

# If args is not set, set it to [-c]
if not args.args:
    args.args = ["-c"]

# Get the directory of this script
script_dir = os.path.dirname(os.path.realpath(__file__))
assignment = sys.argv[1]
test = sys.argv[2]
assignment_number = assignment[1:]
assignment_number = max(2, int(assignment_number))

# Grab the STDLIB
stdlib_dir = f"/u/cs444/pub/stdlib/{assignment_number}.0"
test_dir = f"/u/cs444/pub/assignment_testcases/{assignment}/{test}"

# Check that both the stdlib directory exist and is a directory
if not os.path.exists(stdlib_dir) or not os.path.isdir(stdlib_dir):
    print(f"Error: stdlib directory {stdlib_dir} does not exist")
    sys.exit(1)

# Check that the test directory exist (or is a file)
if not os.path.exists(test_dir):
    print(f"Error: test directory {test_dir} does not exist")
    sys.exit(1)

# Grab the files
files = [test_dir] if os.path.isfile(test_dir) else grab_all_java(test_dir)
files += grab_all_java(stdlib_dir)

# Dump the JSON
vscode_dir = os.path.join(script_dir, "..", ".vscode")
os.makedirs(vscode_dir, exist_ok=True)
with open(os.path.join(vscode_dir, "launch.json"), "w") as f:
    f.write(build_json(f"jcc1 {assignment}/{test}", [*args.args, *files]))
