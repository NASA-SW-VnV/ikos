Install IKOS on Arch Linux
==========================

IKOS can be installed on **[Arch Linux](https://www.archlinux.org/)** by building and installing the *apron* and *ikos* **[AUR](https://aur.archlinux.org/)** packages. See **[Installing packages](https://wiki.archlinux.org/index.php/Arch_User_Repository#Installing_packages)** for detailed instructions.

* https://aur.archlinux.org/packages/apron
* https://aur.archlinux.org/packages/ikos

You are now ready to use IKOS. Go to the section [How to Run IKOS](../../README.md#how-to-run-ikos) in README.md

To be informed on updates by email, click on "Enable notifications" in "Package actions" on the AUR website.

Build IKOS from source on Archlinux
===================================

Here are the steps to install the required dependencies to build IKOS from source on **[Arch Linux](https://www.archlinux.org/)** using **[AUR](https://aur.archlinux.org/)** (Archlinux User Repositories).

First, make sure your system is up-to-date:

```
$ sudo pacman -Syu
```

Now, install the following packages using **pacman**:

```
$ sudo pacman -S base-devel cmake gmp boost boost-libs python python-pygments sqlite llvm llvm-libs clang
```

Now install and build the apron package.

* https://aur.archlinux.org/packages/apron

You are now ready to build IKOS. Go to the section [Build and Install](../../README.md#build-and-install) in README.md
