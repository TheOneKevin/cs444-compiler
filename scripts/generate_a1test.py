import os
import subprocess


def grab_all_java(dir):
    # Recursively grab all java files
    java_files = []
    for root, _, files in os.walk(dir):
        for file in files:
            if file.endswith(".java"):
                java_files.append(os.path.join(root, file))
    return java_files


def get_files(test_path):
    files = [test_path] if os.path.isfile(test_path) else grab_all_java(test_path)
    files += stdlib_files
    return files


def merge_all_files(files):
    merged = ""
    for file in files:
        with open(file, "r") as f:
            tmp = f.read()
            # Escape the file contents to be put into a string
            tmp = tmp.replace("\\", "\\\\")
            tmp = tmp.replace('"', '\\"')
            tmp = tmp.replace("\n", "\\n")
            merged += tmp
            merged += "\\n---\\n"
    return merged


# Get the directory of this script
script_dir = os.path.dirname(os.path.realpath(__file__))
test_dir = "/u/cs444/pub/assignment_testcases/a1/"
stdlib_dir = "/u/cs444/pub/stdlib/2.0"
stdlib_files = grab_all_java(stdlib_dir)
joosc = os.path.join(script_dir, "build", "joosc")

# List test cases files
test_names = os.listdir(test_dir)

# Set of starting lines
invalid_files = [x for x in test_names if x.startswith("Je_")]
valid_files = [x for x in test_names if x not in invalid_files]

for file in valid_files:
    # Run joosc and get return code
    file = os.path.join(test_dir, file)
    ret = subprocess.run(
        [joosc, file, *stdlib_files], stderr=subprocess.PIPE, stdout=subprocess.PIPE
    )
    if ret.returncode != 0:
        print("Failed " + file)

for file in invalid_files:
    # Run joosc and get return code
    file = os.path.join(test_dir, file)
    ret = subprocess.run(
        [joosc, file, *stdlib_files], stderr=subprocess.PIPE, stdout=subprocess.PIPE
    )
    if ret.returncode != 42:
        print("Failed " + file)
