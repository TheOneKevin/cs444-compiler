#!/usr/bin/env python3

import json
import os
import sys


def build_json(name, args):
    return json.dumps(
        {
            "version": "0.2.0",
            "configurations": [
                {
                    "type": "lldb",
                    "request": "launch",
                    "name": name,
                    "program": "${workspaceFolder}/build/jcc1",
                    "args": args,
                    "cwd": "${workspaceFolder}",
                }
            ],
        }
    )


# Recursively grab all java files subroutine
def grab_all_java(dir):
    java_files = []
    for root, _, files in os.walk(dir):
        for file in files:
            if file.endswith(".java"):
                java_files.append(os.path.join(root, file))
    return java_files


# Grab the assignment number from the command line
if len(sys.argv) != 3:
    print(f"Usage: python3 {sys.argv[0]} <assignment> <test>")
    print("Where <test> is the name of the .java file **OR** the directory")
    print(f"Example: python3 {sys.argv[0]} a2 J2_hierachyCheck25")
    sys.exit(1)

# Get the directory of this script
script_dir = os.path.dirname(os.path.realpath(__file__))
assignment = sys.argv[1]
test = sys.argv[2]
assignment_number = assignment[1:]

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
    f.write(build_json(f"jcc1 {assignment}/{test}", files))
