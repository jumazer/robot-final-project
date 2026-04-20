#!/usr/bin/env python3
import sys

DEV = "/dev/rfcomm1"
msg = sys.argv[1] if len(sys.argv) > 1 else "w"

with open(DEV, "wb", buffering=0) as f:
    f.write(msg.encode("ascii"))

print(f"Sent {msg!r} to {DEV}")
