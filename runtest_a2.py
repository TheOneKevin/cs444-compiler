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

# Get the directory of this script
script_dir = os.path.dirname(os.path.realpath(__file__))

# Set up the test directories
test_dir = "/u/cs444/pub/assignment_testcases/a2/"
stdlib_dir = "/u/cs444/pub/stdlib/2.0"
joosc = os.path.join(script_dir, "build", "joosc")

# List test cases files
test_names = os.listdir(test_dir)
stdlib_files = grab_all_java(stdlib_dir)

# Deliniate the valid and invalid files
invalid_files = [x for x in test_names if x.startswith("Je_")]
valid_files = [x for x in test_names if x not in invalid_files]

def run_on_files(files):
    ret = subprocess.run([joosc, *files], stderr=subprocess.PIPE, stdout=subprocess.PIPE)
    return ret.returncode, ret.stdout, ret.stderr

def get_files(test):
    files = [test] if os.path.isfile(test) else grab_all_java(test)
    if os.path.isdir(test) and os.path.isdir(os.path.join(test, "java")):
        pass
    else:
        pass
    files += stdlib_files
    return files

# Test the valid files now

failures = 0

for test in valid_files:
    test = os.path.join(test_dir, test)
    files = get_files(test)
    ret, x, y = run_on_files(files)
    if ret != 0:
        print(f"Running test on: {test}")
        print("Test failed")
        print(f"Return code: {ret}")
        failures += 1
        print(f"Command: {joosc} {' '.join(files)}")

for test in invalid_files:
    test = os.path.join(test_dir, test)
    files = get_files(test)
    ret, x, y = run_on_files(files)
    if ret != 42:
        print(f"Running test on: {test}")
        print("Test failed")
        print(f"Return code: {ret}")
        failures += 1
        print(f"Command: {joosc} {' '.join(files)}")

print(f"Total failures: {failures}/{len(test_names)} or {len(test_names) - failures}/{len(test_names)} passing")
