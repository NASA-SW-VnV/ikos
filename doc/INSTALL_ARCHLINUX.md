Install IKOS dependencies on Archlinux
======================================

Here are the steps to install the required dependencies of IKOS on **[Archlinux](https://www.archlinux.org/)**.

First, make sure your system is up-to-date:

```
$ sudo pacman -Syu
```

Now, install the following packages:

```
$ sudo pacman -S base-devel cmake gmp boost boost-libs python sqlite libedit
```

You will need to install LLVM 4 via **[AUR](https://aur.archlinux.org/)** (Archlinux User Repositories).

First, install **yay**, a tool to install packages from **AUR**: https://github.com/jguer/yay

Then, install LLVM 4 using:

```
$ yay -S llvm40 clang40
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
