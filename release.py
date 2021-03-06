import os
import re
import shutil
import subprocess


def read_lines(filename):
    with open(filename) as f:
        return [s.rstrip("\n") for s in f]


def write_lines(filename, lines):
    with open(filename, "w") as f:
        for s in lines:
            f.write(s + "\n")


def list_rindex(v, x):
    i = len(v)
    while 1:
        i -= 1
        if v[i] == x:
            return i


# version

for s in read_lines("replac.cc"):
    m = re.match('#define version "(.+)"', s)
    if m:
        version = m[1]
if not version:
    print("replac.cc: version not defined")
    exit(1)

# readme

v = read_lines("README.md")
i = v.index("```")
j = list_rindex(v, "```")
assert i < j

p = subprocess.Popen(
    ["./replac.exe", "-h"],
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE,
)
stdout, stderr = p.communicate()
stdout = str(stdout, "utf-8")
stderr = str(stderr, "utf-8")
if stderr:
    print(stderr)
    exit(1)
h = stdout.splitlines()

v[i + 1 : j] = h
write_lines("README.md", v)

# build

subprocess.check_call("build-release.bat")

# zip

d = "replac-" + version
if os.path.exists(d):
    shutil.rmtree(d)
os.mkdir(d)

subprocess.check_call("copy *.exe " + d, shell=1)
subprocess.check_call("copy *.md " + d, shell=1)
subprocess.check_call("copy LICENSE " + d, shell=1)

subprocess.check_call("del *.zip", shell=1)
subprocess.check_call("7z a " + d + ".zip " + d)

shutil.rmtree(d)
