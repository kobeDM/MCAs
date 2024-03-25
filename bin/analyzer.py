#!/usr/bin/python3

import os, sys
import subprocess
import glob

# Check arguments
if len(sys.argv) != 3:
    print("Usage: python3 analyzer.py [data directory] [output directory]")
    sys.exit(1)
datadirpath = sys.argv[1]
outputddirpath = sys.argv[2]
if not os.path.exists(datadirpath):
    print("Error: data directory not found")
    sys.exit(1)
if not os.path.exists(outputddirpath):
    os.makedirs(outputddirpath)

# Make datafile list
datafiles = sorted(glob.glob(datadirpath + "/*.mca"))
with open("datafiles.txt", "w") as f:
    for datafile in datafiles:
        f.write(datafile + "\n")


def run_mca2root():
    cmd = ["root", "-l", "-q", "-b", "source/mca2root.cc"]
    subprocess.run(cmd)


def send_items(items):
    for item in items:
        if os.path.isfile(item):
            os.system("mv " + item + " " + outputddirpath)
        else:
            print("Error: " + item + " is not file or directory")


if __name__ == "__main__":
    print("> mca2root start")
    run_mca2root()
    print("> mca2root done")
    print("---")
    print("> send items")
    items = [
        "datafiles.txt",
        "raw.root",
    ]
    send_items(items)
    print("> done")
