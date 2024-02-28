#!/usr/bin/env python3

import sys
import os

sys.path.append(os.path.join(os.path.dirname(__file__), "common.py"))

from common import *

def fn_more_args(parser: argparse.ArgumentParser):
    parser.add_argument("-d", action='store_true', help="Always dump the launch.json file")

# Parse the arguments
args = get_argparse("Runs jcc1 on a specific testcase", True, fn_more_args)
test_dir, stdlib_dir = get_directories(args.assignment)
test_case = get_testcase(test_dir, args.test)
command = get_joosc_command(args.args, test_case, stdlib_dir)

# Run the command
rc, out, err = run_test_case(command)

print("Command:")
print(subprocess.list2cmdline(command))
print("return code:")
print(rc)
print("stdout:")
if out:
    print(out.decode("utf-8", errors="ignore"))
print("stderr:")
if err:
    print(err.decode("utf-8", errors="ignore"))

if rc != 0 or args.d:
    print("---")
    print("Dumping the launch.json now...")
    # Dump the JSON
    vscode_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", ".vscode")
    os.makedirs(vscode_dir, exist_ok=True)
    with open(os.path.join(vscode_dir, "launch.json"), "w") as f:
        f.write(build_json(f"jcc1", command))

sys.exit(rc)
