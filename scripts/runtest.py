#!/usr/bin/env python3

from functools import reduce
import sys
import os
import argparse

sys.path.append(os.path.join(os.path.dirname(__file__), "common.py"))

from common import *


# Run the tests
def run_test(test, exp_rc):
    global failures
    test_path = os.path.join(test_dir, test)
    cmd = get_joosc_command(args.args, test_path, stdlib_dir)
    ret, _, _ = run_test_case(cmd)
    if ret != exp_rc:
        print(f"{test} failed with return code {ret}, expected {exp_rc}")
        return 1
    return 0

def fn_more_args(parser: argparse.ArgumentParser):
    parser.add_argument("-l", action='store_true', help="List all the test cases and exit")

# Parse the arguments
args = get_argparse("Runs all the marmoset tests for an assignment", False, fn_more_args)
test_dir, stdlib_dir = get_directories(args.assignment)

# Deliniate the valid and invalid files
test_names = os.listdir(test_dir)
if args.l:
    print("\n".join(test_names))
    sys.exit(0)
invalid_files = [x for x in test_names if x.startswith("Je_")]
valid_files = [x for x in test_names if x not in invalid_files]

# Count the failures by running the tests
failures = reduce(
    lambda x, y: x + y,
    [run_test(test, 0) for test in valid_files]
    + [run_test(test, 42) for test in invalid_files],
)

print("---")
print(
    f"Total failures: {failures}/{len(test_names)} or {len(test_names) - failures}/{len(test_names)} passing"
)
