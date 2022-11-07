#!/usr/bin/python3

import os
import subprocess
import sys

sys.path.append("..")
from  run_command import run_command

def main():

    blank_filename = "blank"

    commands = []

    commands.append(
        {
            "args": ["clw", "echo", "CLW"], 
        }
    )

    res = True

    for command in commands:
        with open(blank_filename, "w") as blank_file:
            blank_file.write("")
            blank_file.flush()
        print()
        res = run_command(**command) and res
        print()

    os.remove(blank_filename)

    if res:
        print("All commands returned expected values")
        print()
        return 0
    print("A command returned an unexpected value")
    print()
    return 1

if __name__ == "__main__":
    ret_code = main()
    sys.exit(ret_code)