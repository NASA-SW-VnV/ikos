Install IKOS dependencies on Ubuntu 16.04
=========================================

Here are the steps to install the required dependencies of IKOS on **[Ubuntu 16.04 (Xenial)](http://releases.ubuntu.com/16.04/)**.

First, make sure your system is up-to-date:

```
$ sudo apt-get update
$ sudo apt-get upgrade
```

Then, run the following command:

```
$ sudo apt-get install cmake libgmp-dev libsqlite3-dev libz-dev libedit-dev \
    libboost-dev libboost-program-options-dev libboost-filesystem-dev \
    gcc g++ llvm-4.0 clang-4.0
```

Now, add the LLVM directory in your `PATH` (consider adding this in your `.bashrc`):

```
$ PATH="/usr/lib/llvm-4.0/bin:$PATH"
```

You are now ready to build IKOS. Go to the section [Build and Install IKOS](../README.md#build-and-install-ikos) in README.md
