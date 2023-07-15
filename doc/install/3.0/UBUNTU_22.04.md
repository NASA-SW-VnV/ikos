Install IKOS dependencies on Ubuntu 22.04
=========================================


Here are the steps to install the required dependencies of IKOS on **[Ubuntu 22.04 LTS (Jammy Jellyfish)](http://releases.ubuntu.com/22.04/)**.

First, make sure your system is up-to-date: 

```
$ sudo apt-get update
$ sudo apt-get upgrade
```

Then, run the following commands:

```
$ sudo apt-get install gcc g++ cmake libgmp-dev libboost-dev libboost-filesystem-dev \
    libboost-thread-dev libboost-test-dev python3 python3-pygments libsqlite3-dev libtbb-dev \
    2ibz-dev libedit-dev llvm-14 llvm-14-dev llvm-14-tools clang-14
```

When running cmake to build IKOS, you will need to define `LLVM_CONFIG_EXECUTABLE`:

```
$ cmake \
    -DCMAKE_INSTALL_PREFIX="/path/to/ikos-install-directory" \
    -DLLVM_CONFIG_EXECUTABLE="/usr/bin/llvm-config-14" \
    ..
```

You are now ready to build IKOS. Go to the section [Build and Install](../../../README.md#build-and-install) in README.md
