import json
import os
import sys
import argparse
import subprocess


# Build the json file for debugging from exec path, test name and prog args
def build_json(name, args):
    return json.dumps(
        {
            "version": "0.2.0",
            "configurations": [
                {
                    "type": "lldb",
                    "request": "launch",
                    "name": name,
                    "program": args[0],
                    "args": args[1:],
                    "cwd": "${workspaceFolder}",
                }
            ],
        },
        indent=4,
    )


# Recursively grab all java files subroutine
def grab_all_java(dir):
    java_files = []
    for root, _, files in os.walk(dir):
        for file in files:
            if file.endswith(".java"):
                java_files.append(os.path.join(root, file))
    return java_files


# Gets the test directory and stdlib directory for an assignment
def get_directories(anum: int):
    assignment = f"a{anum}"
    test_dir = f"/u/cs444/pub/assignment_testcases/{assignment}"
    stdlib_dir = f"/u/cs444/pub/stdlib/{str(min(2, anum))}.0"
    # Check that both the stdlib directory exist and is a directory
    if not os.path.exists(stdlib_dir) or not os.path.isdir(stdlib_dir):
        print(f"Error: stdlib directory {stdlib_dir} does not exist")
        sys.exit(1)
    # Check that the test directory exist and is a directory
    if not os.path.exists(test_dir) or not os.path.isdir(test_dir):
        print(f"Error: test directory {test_dir} does not exist")
        sys.exit(1)
    return test_dir, stdlib_dir


# Gets the test case from the test directory
def get_testcase(test_dir: str, test: str):
    ret = os.path.join(test_dir, test)
    # Check that the test directory exist (or is a file)
    if not os.path.exists(ret):
        print(f"Error: test directory {ret} does not exist")
        sys.exit(1)
    return ret


# Grabs the arguments for joosc for subprocess.run to use
def get_joosc_command(args: list[str], test_case: str, stdlib_dir: str):
    script_dir = os.path.dirname(os.path.realpath(__file__))
    files = [test_case] if os.path.isfile(test_case) else grab_all_java(test_case)
    binary = os.path.join(script_dir, "..", "build", "jcc1")
    custom_args = [*args]
    if "JOOSC" in os.environ:
        files += grab_all_java(stdlib_dir)
        binary = os.environ["JOOSC"]
    else:
        custom_args += ["--stdlib", stdlib_dir]
    return [binary, *custom_args, *files]


# Run joosc and get return code subroutine
def run_test_case(args: list[str]):
    ret = subprocess.run(args, stderr=subprocess.PIPE, stdout=subprocess.PIPE)
    return ret.returncode, ret.stdout, ret.stderr


# Parse the command line arguments
def get_argparse(desc: str, single_test: bool):
    parser = argparse.ArgumentParser(description=desc)
    parser.add_argument("assignment", help="The assignment number (a1, a2, etc)")
    if single_test:
        parser.epilog = (
            "Where <test> is the name of the .java file **OR** the directory"
            "Example: python3 {sys.argv[0]} a2 J2_hierachyCheck25"
        )
        parser.add_argument("test", help="The test case to run")
    parser.add_argument("args", nargs="*", help="Additional arguments to pass to jcc1")
    args = parser.parse_args()
    # If custom args is not set, set it to [-c]
    if not args.args:
        args.args = ["-c"]
    # Change the assignment number to an integer
    args.assignment = int(args.assignment[1:])
    return args
