#! /usr/bin/env bash

cd "$(dirname "$0")"
qemu-system-x86_64 -L . -m 64 -fda ./Disk.img -M pc
