#!/usr/bin/env bash

sudo rfcomm release 1 2>/dev/null
sudo rm -f /dev/rfcomm1
sudo rfcomm bind 1 00:14:03:05:09:9E 1

if [ -c /dev/rfcomm1 ]; then
    echo "OK: /dev/rfcomm1 is a character device"
    ls -l /dev/rfcomm1
else
    echo "ERROR: /dev/rfcomm1 is not a character device"
fi