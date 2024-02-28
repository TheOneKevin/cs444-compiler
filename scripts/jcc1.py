#!/usr/bin/env python3

import sys
import os

sys.path.append(os.path.join(os.path.dirname(__file__), "common.py"))

from common import *

# Parse the arguments
args = get_argparse("Runs jcc1 on a specific testcase", True)
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
print(out.decode("utf-8", errors="ignore"))
print("stderr:")
if err:
    print(err.decode("utf-8", errors="ignore"))
sys.exit(rc)
