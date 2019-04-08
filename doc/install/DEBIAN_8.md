Install IKOS dependencies on Debian Jessie
==========================================

Here are the steps to install the required dependencies of IKOS on **[Debian 8 (Jessie)](https://www.debian.org/releases/jessie/)**.

First, make sure your system is up-to-date:

```
$ sudo apt-get update
$ sudo apt-get upgrade
```

Now, you will need to add the LLVM repository to your apt `sources.list`:

```
$ echo "deb http://apt.llvm.org/jessie/ llvm-toolchain-jessie-8 main" | sudo tee -a /etc/apt/sources.list
```

You also need to trust the LLVM repository key:

```
$ wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
```

You will need a more recent version of CMake that is not available on Debian 8.

Since it is only needed during configuration, you can install it in a temporary directory:

```
$ cd /tmp
$ wget https://github.com/Kitware/CMake/releases/download/v3.14.1/cmake-3.14.1-Linux-x86_64.tar.gz
$ tar xf cmake-3.14.1-Linux-x86_64.tar.gz
$ PATH="/tmp/cmake-3.14.1-Linux-x86_64/bin:$PATH"
```

Finally, run the following commands to install the remaining dependencies:

```
$ sudo apt-get update
$ sudo apt-get install gcc g++ make libgmp-dev libboost-dev libboost-filesystem-dev \
    libboost-test-dev python python-pygments libsqlite3-dev libz-dev libedit-dev \
    llvm-8 llvm-8-dev llvm-8-tools clang-8
```

When running cmake to build IKOS, you will need to define `LLVM_CONFIG_EXECUTABLE` and `LLVM_TEMPORARILY_ALLOW_OLD_TOOLCHAIN`:

```
$ cmake \
    -DCMAKE_INSTALL_PREFIX="/path/to/ikos-install-directory" \
    -DLLVM_CONFIG_EXECUTABLE="/usr/lib/llvm-8/bin/llvm-config" \
    -DLLVM_TEMPORARILY_ALLOW_OLD_TOOLCHAIN=ON \
    ..
```

You are now ready to build IKOS. Go to the section [Build and Install](../../README.md#build-and-install) in README.md
