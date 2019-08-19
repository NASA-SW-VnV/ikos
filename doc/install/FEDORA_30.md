Install IKOS dependencies on Fedora 30
======================================

Here are the steps to install the required dependencies of IKOS on **[Fedora 30](https://getfedora.org/)**.

First, make sure your system is up-to-date:

```
$ sudo dnf update
```

Now, install the following packages:

```
$ sudo dnf install gcc gcc-c++ make cmake gmp-devel boost-devel \
    python python-pygments sqlite-devel zlib-devel \
    llvm llvm-devel llvm-static clang
```

You are now ready to build IKOS. Go to the section [Build and Install](../../README.md#build-and-install) in README.md
