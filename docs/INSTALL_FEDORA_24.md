INSTALL IKOS DEPENDENCIES ON FEDORA 24
======================================

Here are the steps to install the required dependencies of IKOS on **Fedora 24**.

First, make sure your system is up-to-date:

```
$ sudo dnf update
```

Now, install the following packages:

```
$ sudo dnf install make cmake python gmp-devel sqlite-devel zlib-devel libedit-devel boost-devel gcc gcc-c++ llvm llvm-devel llvm-static clang
```

You are now ready to build IKOS. Go to the next section of the `README.md`.
