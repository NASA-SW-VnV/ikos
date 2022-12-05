Install IKOS on Arch Linux
==========================

**NOTE: These instructions are for IKOS 3.0 with LLVM 9 and are not actively maintained. Please see the main [README.md](../../../README.md)**

Here are the steps to install IKOS on **[Arch Linux](https://www.archlinux.org/)** using **[AUR](https://aur.archlinux.org/)** (Arch User Repository).

First, make sure your system is up-to-date:

```
$ sudo pacman -Syu
```

Then, install the *apron* and *ikos* **[AUR](https://aur.archlinux.org/)** packages. See **[Installing packages](https://wiki.archlinux.org/index.php/Arch_User_Repository#Installing_packages)** for detailed instructions.

* https://aur.archlinux.org/packages/apron
* https://aur.archlinux.org/packages/ikos

To be informed on updates by email, click on "Enable notifications" in "Package actions" on the AUR website.

You are now ready to use IKOS. Go to the section [How to Run IKOS](../../../README.md#how-to-run-ikos) in README.md

Build IKOS from source on Arch Linux
====================================

Here are the steps to install the required dependencies to build IKOS from source on **[Arch Linux](https://www.archlinux.org/)** using **[AUR](https://aur.archlinux.org/)** (Arch User Repository).

First, make sure your system is up-to-date:

```
$ sudo pacman -Syu
```

Now, install the following packages using **pacman**:

```
$ sudo pacman -S base-devel cmake gmp boost boost-libs python python-pygments sqlite intel-tbb llvm llvm-libs clang
```

Then, install the *apron-ppl-svn* **[AUR](https://aur.archlinux.org/)** package:

* https://aur.archlinux.org/packages/apron-ppl-svn

You are now ready to build IKOS. Go to the section [Build and Install](../../../README.md#build-and-install) in README.md
