#!/bin/bash

rm -rf build
mkdir build
cd build
cmake ..
make
arm-none-eabi-objcopy -O binary Bootloader.elf Bootloader.bin
cd --

