#!/usr/bin/env python3

from functools import reduce
import sys
import os
import argparse

sys.path.append(os.path.join(os.path.dirname(__file__), "common.py"))

from common import *


# Run the tests
def run_test(test: str) -> int:
    print(f"Running {test}")
    test_path = os.path.join(test_dir, test)
    cmd = get_joosc_command(args.args, test_path, stdlib_dir)
    _, _, stderr = run_test_case(cmd)
    print(stderr.decode("utf-8", errors="ignore"))
    print("=" * 80)


# Parse the arguments
args = get_argparse("Runs all the marmoset tests for an assignment", False)
test_dir, stdlib_dir = get_directories(args.assignment)

# Deliniate the valid and invalid files
test_names = os.listdir(test_dir)
invalid_files = [x for x in test_names if x.startswith("Je_")]

# Sort the files alphabetically
invalid_files.sort()
for test in invalid_files:
    run_test(test)
