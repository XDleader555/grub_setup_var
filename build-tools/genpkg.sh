#!/bin/bash

echo "Building grub..."
./genpatch.sh
cd ../
makepkg -f
