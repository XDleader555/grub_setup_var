# setup_var_grub
Modified grub-git [PKGBUILD](https://wiki.archlinux.org/index.php/PKGBUILD) containing custom setup_var patch.
This version of the setup_var patch allows users to edit efi variable stores outside of "Setup" for manufacturers with different naming conventions.

I am not responsible for any losses or damages caused by the use of this program. USE AT YOUR OWN RISK.

# Usage
This modification uses the same syntax, with the addition of the storevariable.
```
setup_var storename offset [setval]
```

In the typical use the storename will be "Setup"
```
$ setup_var Setup
```

# Build Notes
You can find pre-compiled binaries ready for use in [releases](https://github.com/XDleader555/grub_setup_var/releases)

If you want to build and install this package on Arch Linux, simply run
```
$ makepkg -si
```

otherwise, for other linux distros, you can open up the PKGBUILD in your favorite text editor and follow the build process.

To generate a standalone efi shell after building the project, from the build directory run
```
$ ./build_x86_64-efi/grub-mkstandalone -d ./build_x86_64-efi/grub-core/ -O x86_64-efi -o grubx64.efi
```
