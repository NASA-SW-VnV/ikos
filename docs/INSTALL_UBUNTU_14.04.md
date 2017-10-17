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
$ sudo add-apt-repository -y "deb http://apt.llvm.org/trusty/ llvm-toolchain-trusty-4.0 main"
```

Note: [ppa:ubuntu-toolchain-r](https://launchpad.net/~ubuntu-toolchain-r/+archive/ubuntu/test) provides gcc 7.x and [apt.llvm.org](http://apt.llvm.org/) provides llvm 4.0.x

Then, run the following commands:

```
$ sudo apt-get update
$ sudo apt-get install cmake libgmp-dev libsqlite3-dev libz-dev libedit-dev \
    libboost1.55-dev libboost-program-options1.55-dev libboost-filesystem1.55-dev \
    gcc-7 g++-7 llvm-4.0 clang-4.0
```

Now, add the LLVM directory to your `PATH` (consider adding this in your `.bashrc`):

```
$ PATH="/usr/lib/llvm-4.0/bin:$PATH"
```

Define `CC` and `CXX` to use gcc 7 instead of the default compiler:

```
$ export CC=gcc-7
$ export CXX=g++-7
```

You are now ready to build IKOS. Go to the section [Build and Install IKOS](../README.md#build-and-install-ikos) in README.md
