#!/usr/bin/env python3

import os, stat, sys

RFCOMM_FILE = "/dev/rfcomm1"


def main():
    st = os.stat(RFCOMM_FILE)
    if not stat.S_ISCHR(st.st_mode):
        print(f"{RFCOMM_FILE} file is not a character device")
        sys.exit(1)


    while True:
        cmd = input("Please enter a command: ")
        if not cmd.isalpha():
            print(f"Bad cmd: {cmd}, please enter another")
            continue

        cmd = cmd + "\n"
        cmd = cmd.encode('ascii')

        with open(RFCOMM_FILE, "wb", buffering=0) as f:
            f.write(cmd)

        print(f"Sent command: {cmd} to bluetooth module HC-05")


if __name__ == "__main__":
    main()