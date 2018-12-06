Install IKOS dependencies on Ubuntu 14.04
=========================================

Here are the steps to install the required dependencies of IKOS on **[Ubuntu 14.04 (Trusty)](http://releases.ubuntu.com/14.04/)**.

First, make sure your system is up-to-date:

```
$ sudo apt-get update
$ sudo apt-get upgrade
```

You will need to add some repositories to your apt `sources.list`. Install the package `software-properties-common` that provides `add-apt-repository`:

```
$ sudo apt-get install software-properties-common
```

Now, add the following repositories:

```
$ sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
$ sudo add-apt-repository -y "deb http://apt.llvm.org/trusty/ llvm-toolchain-trusty-7 main"
```

Note: [ppa:ubuntu-toolchain-r](https://launchpad.net/~ubuntu-toolchain-r/+archive/ubuntu/test) provides gcc 8.x and [apt.llvm.org](http://apt.llvm.org/) provides llvm 7.0.x

Then, run the following commands:

```
$ sudo apt-get update
$ sudo apt-get install gcc-8 g++-8 cmake libgmp-dev libboost1.55-dev \
    libboost-filesystem1.55-dev libboost-test1.55-dev python python-pygments \
    libsqlite3-dev libz-dev libedit-dev llvm-7 llvm-7-dev clang-7
```

When running cmake to build IKOS, you will need to define `CMAKE_C_COMPILER`, `CMAKE_CXX_COMPILER` and `LLVM_CONFIG_EXECUTABLE`:

```
$ cmake \
    -DCMAKE_INSTALL_PREFIX="/path/to/ikos-install-directory" \
    -DCMAKE_C_COMPILER="gcc-8" \
    -DCMAKE_CXX_COMPILER="g++-8" \
    -DLLVM_CONFIG_EXECUTABLE="/usr/lib/llvm-7/bin/llvm-config" \
    ..
```

You are now ready to build IKOS. Go to the section [Build and Install](../README.md#build-and-install) in README.md
