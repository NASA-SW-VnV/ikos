Install IKOS dependencies on Ubuntu 20.04
=========================================

**NOTE: These instructions are for IKOS 3.0 with LLVM 9 and are not actively maintained. Please see the main [README.md](../../../README.md)**

Here are the steps to install the required dependencies of IKOS on **[Ubuntu 20.04 LTS (Focal Fossa)](http://releases.ubuntu.com/20.04/)**.

First, make sure your system is up-to-date: 

```
$ sudo apt-get update
$ sudo apt-get upgrade
```

Then, run the following commands:

```
$ sudo apt-get install gcc g++ cmake libgmp-dev libboost-dev libboost-filesystem-dev \
    libboost-thread-dev libboost-test-dev python3 python3-pygments libsqlite3-dev libtbb-dev \
    libz-dev libedit-dev llvm-9 llvm-9-dev llvm-9-tools clang-9
```

When running cmake to build IKOS, you will need to define `LLVM_CONFIG_EXECUTABLE`:

```
$ cmake \
    -DCMAKE_INSTALL_PREFIX="/path/to/ikos-install-directory" \
    -DLLVM_CONFIG_EXECUTABLE="/usr/bin/llvm-config-9" \
    ..
```

You are now ready to build IKOS. Go to the section [Build and Install](../../../README.md#build-and-install) in README.md
