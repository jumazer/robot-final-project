#!/usr/bin/env python3
import sys
import os, stat, sys

DEV = "/dev/rfcomm1"


def main():
    st = os.stat(DEV)
    if not stat.S_ISCHR(st.st_mode):
        print(f"{DEV} is not a character device")
        sys.exit(1)


    with open(DEV, "wb", buffering=0) as f:
        f.write(b"w")

    print(f"Sent 'w' to bluetooth module HC-05")


if __name__ == "__main__":
    main()