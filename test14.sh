#!/bin/sh

make EXAMPLE=14_bootloader2

cd 14_bootloader2/hostutil
make && ./out/hostutil
