Install IKOS on Archlinux
=========================

Here are the steps to install IKOS on **[Archlinux](https://www.archlinux.org/)** using **[AUR](https://aur.archlinux.org/)** (Archlinux User Repositories).

First, make sure your system is up-to-date:

```
$ sudo pacman -Syu
```

Now, install IKOS using your favorite **AUR** helper:

```
$ yay -S ikos
```

You are now ready to use IKOS. Go to the section [How to Run IKOS](../README.md#how-to-run-ikos) in README.md

Build IKOS from source on Archlinux
===================================

Here are the steps to install the required dependencies to build IKOS from source on **[Archlinux](https://www.archlinux.org/)** using **[AUR](https://aur.archlinux.org/)** (Archlinux User Repositories).

First, make sure your system is up-to-date:

```
$ sudo pacman -Syu
```

Now, install the following packages using your favorite **AUR** helper:

```
$ yay -S base-devel cmake gmp boost boost-libs python sqlite libedit llvm40 clang40
```

Note that this may take some time.

When running cmake to build IKOS, you will need to define `LLVM_CONFIG_EXECUTABLE`:

```
cmake \
    -DCMAKE_INSTALL_PREFIX="/path/to/ikos-install-directory" \
    -DLLVM_CONFIG_EXECUTABLE="/usr/bin/llvm-config-4.0" \
    ..
```

You are now ready to build IKOS. Go to the section [Build and Install](../README.md#build-and-install) in README.md
