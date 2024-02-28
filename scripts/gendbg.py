#!/usr/bin/env python3

import sys
import os

sys.path.append(os.path.join(os.path.dirname(__file__), "common.py"))

from common import *

# Parse the arguments
args = get_argparse("Generate a launch.json file for vscode", True)
test_dir, stdlib_dir = get_directories(args.assignment)
test_case = get_testcase(test_dir, args.test)
command = get_joosc_command(args.args, test_dir, stdlib_dir)

# Dump the JSON
vscode_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", ".vscode")
os.makedirs(vscode_dir, exist_ok=True)
with open(os.path.join(vscode_dir, "launch.json"), "w") as f:
    f.write(build_json(f"jcc1", command))
