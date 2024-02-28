#!/usr/bin/env python3

import sys
import os
import shlex

sys.path.append(os.path.join(os.path.dirname(__file__), "common.py"))

from common import build_json

print("This script will generate a launch.json based the command you give.")
command = input("Paste the command below:\n> ")

# Parse the command into a list, using shlex.split to handle quotes
command = shlex.split(command)

# Dump the JSON
vscode_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", ".vscode")
os.makedirs(vscode_dir, exist_ok=True)
with open(os.path.join(vscode_dir, "launch.json"), "w") as f:
    f.write(build_json(f"custom jcc1", command))
print("Done!")
