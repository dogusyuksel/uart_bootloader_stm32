#!/bin/bash

rm -rf build
mkdir build
cd build
cmake ..
make
arm-none-eabi-objcopy -O binary Application.elf Application.bin
cd --

