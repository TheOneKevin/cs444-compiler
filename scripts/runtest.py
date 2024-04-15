#!/usr/bin/env python3

from functools import reduce
import sys
import os
import argparse

sys.path.append(os.path.join(os.path.dirname(__file__), "common.py"))

from common import *

num_crashes = 0
num_codegen_errors = 0
ind = "⢎⡰⢎⡡⢎⡑⢎⠱⠎⡱⢊⡱⢌⡱⢆⡱"


# Run the tests 
def run_test(test: str, exp_rc: int) -> int:
    global num_crashes
    global ind
    print(f"[{ind[0]}{ind[1]}] Running {test:65.65}", end="\r", flush=True)
    ind = ind[2:] + ind[:2]  # Rotate the indicator
    test_path = os.path.join(test_dir, test)
    cmd = get_joosc_command(args.args, test_path, stdlib_dir)
    ret, _, _ = run_test_case(cmd)
    print(" " * 80, end="\r", flush=True)
    if ret != 0 and ret != 42 and ret != 43:
        num_crashes += 1
        print(f"{test} crashed with return code {ret}")
        return 1
    if ret != exp_rc:
        print(f"{test} failed with return code {ret}, expected {exp_rc}")
        return 1
    return 0

def run_codegen(test: str, exp_rc: int) -> int:
    global num_codegen_errors
    script_dir = os.path.dirname(os.path.realpath(__file__))
    _, stdlib_dir = get_directories(args.assignment)
    build_dir = os.path.join(script_dir, '..', 'build')
    output_file = os.path.join(build_dir, 'output/output.s')
    runtime_file = os.path.join(stdlib_dir, 'runtime.o')
    if not os.path.isfile(output_file):
        num_codegen_errors += 1
        print(f"{test} failed: Output.s not found")
        return 1
    cmd = ["/u/cs444/bin/nasm", "-O1", "-f", "elf", "-g", "-F", "dwarf", output_file]
    ret = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if ret.returncode != 0:
        num_codegen_errors += 1
        print(f"{test} failed: Output.s failed to assemble")
        return 1
    output_file = os.path.join(build_dir, 'output/output.o')
    cmd = ["ld", "-melf_i386", "-o", "main", output_file, runtime_file]
    ret = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if ret.returncode != 0:
        num_codegen_errors += 1
        print(ret.stderr.decode())
        print(f"{test} failed: Linking failed")
        return 1
    cmd = ["./main"]
    ret = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if (ret.returncode != exp_rc):
        print(f"CODEGEN: {test} failed with return code {ret.returncode}, expected {exp_rc}")
        return 1
    return 0

def fn_more_args(parser: argparse.ArgumentParser):
    parser.add_argument(
        "-l", action="store_true", help="List all the test cases and exit"
    )


# Parse the arguments
args = get_argparse(
    "Runs all the marmoset tests for an assignment", False, fn_more_args
)
test_dir, stdlib_dir = get_directories(args.assignment)

# Deliniate the valid and invalid files
test_names = os.listdir(test_dir)
if args.l:
    print("\n".join(test_names))
    sys.exit(0)

# Run the tests for a5 or a6
if (args.assignment == 5 or args.assignment == 6):
    invalid_files = [x for x in test_names if x.startswith("J1e_")]
    valid_files = [x for x in test_names if x not in invalid_files]

    valid_failures = 0
    for test in valid_files:
        test_ret = run_test(test, 0)
        if (test_ret != 0):
            valid_failures += 1
        else:
            valid_failures += run_codegen(test, 0)
    invalid_failures = 0
    for test in invalid_files:
        test_ret = run_test(test, 0)
        if (test_ret != 0):
            invalid_failures += 1
        else:
            invalid_failures += run_codegen(test, 13)
    failures = valid_failures + invalid_failures
    print(
        f"Total failures: {failures}/{len(test_names)}"
        f" or {len(test_names) - failures}/{len(test_names)} passing"
    )
    print(f"Crashed on {num_crashes}/{len(test_names)} tests")
    print(f"Failed to generate code on {num_codegen_errors}/{len(test_names)} tests")
    print(f"Exception is not thrown on {invalid_failures}/{len(invalid_files)} error tests")
    
# Run the tests for A1-A4
else:
    invalid_files = [x for x in test_names if x.startswith("Je_")]
    warning_files = [x for x in test_names if x.startswith("Jw_")]
    valid_files = [x for x in test_names if x not in invalid_files and x not in warning_files]

    # Sort the files alphabetically
    valid_files.sort()
    warning_files.sort()
    invalid_files.sort()
    # Calculate the number of failures
    valid_failures = sum([run_test(test, 0) for test in valid_files])
    print("---")
    invalid_failures = sum([run_test(test, 42) for test in invalid_files])
    print("---")
    warning_failures = sum([run_test(test, 43) for test in warning_files])
    failures = valid_failures + invalid_failures + warning_failures

    # Print the summary report
    print(
        f"Total failures: {failures}/{len(test_names)}"
        f" or {len(test_names) - failures}/{len(test_names)} passing"
    )
    print(f"Incorrectly rejected {valid_failures}/{len(valid_files)} valid tests")
    print(f"Incorrectly accepted {invalid_failures}/{len(invalid_files)} invalid tests")
    print(f"No warnings issued on {warning_failures}/{len(warning_files)} tests")
    print(f"Crashed on {num_crashes}/{len(test_names)} tests")
