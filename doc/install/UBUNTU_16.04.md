Install IKOS dependencies on Ubuntu 16.04
=========================================

Here are the steps to install the required dependencies of IKOS on **[Ubuntu 16.04 (Xenial)](http://releases.ubuntu.com/16.04/)**.

First, make sure your system is up-to-date:

```
$ sudo apt-get update
$ sudo apt-get upgrade
```

Now, you will need to add the LLVM repository to your apt `sources.list`:

```
$ echo "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-7 main" | sudo tee -a /etc/apt/sources.list
```

Then, run the following commands:

```
$ sudo apt-get update
$ sudo apt-get install gcc g++ cmake libgmp-dev libboost-dev libboost-filesystem-dev \
    libboost-test-dev python python-pygments libsqlite3-dev libz-dev libedit-dev \
    llvm-7 llvm-7-dev clang-7
```

When running cmake to build IKOS, you will need to define `LLVM_CONFIG_EXECUTABLE`:

```
$ cmake \
    -DCMAKE_INSTALL_PREFIX="/path/to/ikos-install-directory" \
    -DLLVM_CONFIG_EXECUTABLE="/usr/lib/llvm-7/bin/llvm-config" \
    ..
```

You are now ready to build IKOS. Go to the section [Build and Install](../../README.md#build-and-install) in README.md
