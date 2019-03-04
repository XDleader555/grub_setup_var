#!/bin/bash

cp setup_var.c ../src/grub/grub-core/commands/efi/setup_var.c
cd ../src/grub/build_x86_64-efi/grub-core/
make -j8

cd ../../../../build-tools/
./genstandalone.sh
