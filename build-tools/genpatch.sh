#!/bin/bash

echo "Generating patch for release..."
cd ./patch
mkdir -p ./a/grub-core/commands/efi
cp -r a b
cd ..

cp ./setup_var.c ./patch/b/grub-core/commands/efi/
cp ./Makefile.core.def ./patch/b/grub-core/
cd ./patch
diff -Naur a/ b/ > ../setup_var.patch

rm -r b
cd ..
