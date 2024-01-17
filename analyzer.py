#!/usr/bin/python3

import os, sys
import subprocess
import glob

datadirpath = sys.argv[1]
if not os.path.exists(datadirpath):
    print("Error: data directory not found")
    sys.exit(1)

datafiles = glob.glob(datadirpath + "/*.mca")
with open("datafiles.txt", "w") as f:
    for datafile in datafiles:
        f.write(datafile + "\n")


def run_makeTree():
    cmd = ["root", "-l", "-q", "source/mca2root.cc"]
    subprocess.run(cmd)


def run_draw():
    cmd = ["root", "-l", "-q", "source/draw.cc"]
    subprocess.run(cmd)


if __name__ == "__main__":
    print("makeTree start")
    run_makeTree()
    print("draw start")
    run_draw()
