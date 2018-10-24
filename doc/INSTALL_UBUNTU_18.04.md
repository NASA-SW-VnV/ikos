Install IKOS dependencies on Ubuntu 18.04
=========================================

Here are the steps to install the required dependencies of IKOS on **[Ubuntu 18.04 (Bionic Beaver)](http://releases.ubuntu.com/18.04/)**.

First, make sure your system is up-to-date:

```
$ sudo apt-get update
$ sudo apt-get upgrade
```

Then, run the following command:

```
$ sudo apt-get install gcc g++ cmake libgmp-dev libboost-dev libboost-filesystem-dev \
    libboost-test-dev python python-pygments libsqlite3-dev libz-dev libedit-dev \
    llvm-4.0 llvm-4.0-dev clang-4.0
```

Now, add the LLVM directory in your `PATH` (consider adding this in your `.bashrc`):

```
$ PATH="/usr/lib/llvm-4.0/bin:$PATH"
```

You are now ready to build IKOS. Go to the section [Build and Install](../README.md#build-and-install) in README.md
