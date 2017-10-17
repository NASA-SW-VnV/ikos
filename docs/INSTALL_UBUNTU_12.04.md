Install IKOS dependencies on Ubuntu 12.04
=========================================

Here are the steps to install the required dependencies of IKOS on **[Ubuntu 12.04 (Precise)](http://releases.ubuntu.com/12.04/)**.

First, make sure your system is up-to-date:

```
$ sudo apt-get update
$ sudo apt-get upgrade
```

You will need to add some repositories to your apt `sources.list`. Install the package `python-software-properties` that provides `add-apt-repository`:

```
$ sudo apt-get install python-software-properties
```

Now, add the following repositories:

```
$ sudo add-apt-repository -y ppa:george-edison55/precise-backports
$ sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
$ sudo add-apt-repository -y ppa:boost-latest/ppa
$ sudo add-apt-repository -y "deb http://apt.llvm.org/precise/ llvm-toolchain-precise-4.0 main"
```

Notes:
* [ppa:george-edison55](https://launchpad.net/~george-edison55/+archive/ubuntu/cmake-3.x) provides cmake 3.2.x
* [ppa:ubuntu-toolchain-r](https://launchpad.net/~ubuntu-toolchain-r/+archive/ubuntu/test) provides gcc 6.x
* [ppa:boost-latest](https://launchpad.net/~boost-latest/+archive/ubuntu/ppa) provides boost 1.55.0
* [apt.llvm.org](http://apt.llvm.org/) provides llvm 4.0.x

Then, run the following commands:

```
$ sudo apt-get update
$ sudo apt-get install cmake libgmp-dev libsqlite3-dev libz-dev libedit-dev \
    libboost1.55-dev libboost-program-options1.55-dev libboost-filesystem1.55-dev \
    gcc-6 g++-6 llvm-4.0 clang-4.0
```

Now, add the LLVM directory to your `PATH` (consider adding this in your `.bashrc`):

```
$ PATH="/usr/lib/llvm-4.0/bin:$PATH"
```

Define `CC` and `CXX` to use gcc 6 instead of the default compiler:

```
$ export CC=gcc-6
$ export CXX=g++-6
```

You are now ready to build IKOS. Go to the section [Build and Install IKOS](../README.md#build-and-install-ikos) in README.md
