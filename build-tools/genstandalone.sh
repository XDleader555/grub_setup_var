#!/bin/sh

../src/grub/build_x86_64-efi/grub-mkstandalone -d ./grub-git/src/grub/build_x86_64-efi/grub-core/ -O x86_64-efi -o grubx64.efi
../src/grub/build_x86_64-efi/grub-mkstandalone -d ./grub-git/src/grub/build_i386-efi/grub-core/ -O i386-efi -o grubi386.efi
