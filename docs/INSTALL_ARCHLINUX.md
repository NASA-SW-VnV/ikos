INSTALL IKOS DEPENDENCIES ON ARCHLINUX
======================================

Here are the steps to install the required dependencies of IKOS on **Archlinux**.

First, make sure your system is up-to-date:

```
$ sudo pacman -Syu
```

Now, install the following packages:

```
$ sudo pacman -S base-devel cmake gmp boost boost-libs python sqlite llvm llvm-libs clang
```

You are now ready to build IKOS. Go to the next section of the `README.md`.
