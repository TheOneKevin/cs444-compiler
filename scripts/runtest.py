#!/usr/bin/env python3

import os
import subprocess
import sys

# Grab the assignment number from the command line
if len(sys.argv) != 2:
    print(f"Usage: python3 {sys.argv[0]} <assignment>")
    sys.exit(1)
assignment = sys.argv[1]


# Recursively grab all java files subroutine
def grab_all_java(dir):
    java_files = []
    for root, _, files in os.walk(dir):
        for file in files:
            if file.endswith(".java"):
                java_files.append(os.path.join(root, file))
    return java_files


# Run joosc and get return code subroutine
def run_on_files(files):
    ret = subprocess.run(
        [joosc, *files], stderr=subprocess.PIPE, stdout=subprocess.PIPE
    )
    return ret.returncode, ret.stdout, ret.stderr


# Get the files subroutine
def get_files(test):
    files = [test] if os.path.isfile(test) else grab_all_java(test)
    files += stdlib_files
    return files


# Get the directory of this script
script_dir = os.path.dirname(os.path.realpath(__file__))

# Set up the test directories
test_dir = f"/u/cs444/pub/assignment_testcases/{assignment}/"
stdlib_dir = "/u/cs444/pub/stdlib/2.0"
joosc = os.path.join(script_dir, "..", "build", "jcc1")

# List test cases files
test_names = os.listdir(test_dir)
stdlib_files = grab_all_java(stdlib_dir)

# Deliniate the valid and invalid files
invalid_files = [x for x in test_names if x.startswith("Je_")]
valid_files = [x for x in test_names if x not in invalid_files]

failures = 0

for test in valid_files:
    test = os.path.join(test_dir, test)
    files = get_files(test)
    ret, x, y = run_on_files(files)
    if ret != 0:
        print(f"Running test on: {test}")
        print("Test failed")
        print(f"Return code: {ret}")
        failures += 1
        print(f"Command: {joosc} {' '.join(files)}")
        print("\n===========================================================\n")

for test in invalid_files:
    test = os.path.join(test_dir, test)
    files = get_files(test)
    ret, x, y = run_on_files(files)
    if ret == 0:
        print(f"Running test on: {test}")
        print("Test failed")
        print(f"Return code: {ret}")
        failures += 1
        print(f"Command: {joosc} {' '.join(files)}")
        print("\n===========================================================\n")

print(
    f"Total failures: {failures}/{len(test_names)} or {len(test_names) - failures}/{len(test_names)} passing"
)
