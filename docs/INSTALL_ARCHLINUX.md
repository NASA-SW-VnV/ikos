Install IKOS dependencies on Archlinux
======================================

Here are the steps to install the required dependencies of IKOS on **[Archlinux](https://www.archlinux.org/)**.

First, make sure your system is up-to-date:

```
$ sudo pacman -Syu
```

Now, install the following packages:

```
$ sudo pacman -S base-devel cmake gmp boost boost-libs python2 python sqlite libedit
```

You will need to install LLVM 4 via **[AUR](https://aur.archlinux.org/)** (Archlinux User Repositories).

First, install **yaourt**, a tool to install packages from **AUR**: https://archlinux.fr/yaourt-en

Now, to install LLVM 4, you will need to import the following GPG key in your keyring:

```
$ gpg --recv-key 8F0871F202119294
```

Finally, install LLVM 4 using:

```
$ yaourt -S llvm40
```

Note that this may take some time.

You are now ready to build IKOS. Go to the section [Build and Install IKOS](../README.md#build-and-install-ikos) in README.md
