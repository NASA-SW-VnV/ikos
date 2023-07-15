Install IKOS on Arch Linux
==========================

**NOTE: These instructions are for IKOS 3.x with LLVM 14 and are not actively maintained. Please see the main [README.md](../../../README.md)**

Here are the steps to install IKOS on **[Arch Linux](https://www.archlinux.org/)** using **[AUR](https://aur.archlinux.org/)** (Arch User Repository).

First, make sure your system is up-to-date:

```
$ sudo pacman -Syu
```

Then, install the IKOS dependency apron *apron*, and either the *ikos* package containing a recent IKOS release or the *ikos-git* package to build the latest development version of IKOS from the **[AUR](https://aur.archlinux.org/)**. See **[Installing packages](https://wiki.archlinux.org/index.php/Arch_User_Repository#Installing_packages)** for detailed instructions.

If the [*ikos* AUR package](https://aur.archlinux.org/packages/ikos) is out of date, you can flag it to notify the maintainer.

* https://aur.archlinux.org/packages/apron
* https://aur.archlinux.org/packages/ikos or https://aur.archlinux.org/packages/ikos-git

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

Then, install the *apron-git* **[AUR](https://aur.archlinux.org/)** package:

* https://aur.archlinux.org/packages/apron-git

You are now ready to build IKOS. Go to the section [Build and Install](../../../README.md#build-and-install) in README.md
