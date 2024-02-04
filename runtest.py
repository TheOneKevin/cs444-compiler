import os
import subprocess

# Get the directory of this script
script_dir = os.path.dirname(os.path.realpath(__file__))

test_dir = "/u/cs444/pub/assignment_testcases/a1/"
joosc = os.path.join(script_dir, "build", "joosc")

# List test cases files
test_names = os.listdir(test_dir)

# Set of starting lines
invalid_files = [x for x in test_names if x.startswith("Je_")]
valid_files = [x for x in test_names if x not in invalid_files]

for file in valid_files:
    # Run joosc and get return code
    file = os.path.join(test_dir, file)
    ret = subprocess.run([joosc, file], stderr=subprocess.PIPE, stdout=subprocess.PIPE)
    if ret.returncode != 0:
        print("Failed " + file)

for file in invalid_files:
    # Run joosc and get return code
    file = os.path.join(test_dir, file)
    ret = subprocess.run([joosc, file], stderr=subprocess.PIPE, stdout=subprocess.PIPE)
    if ret.returncode != 42:
        print("Failed " + file)
