Install IKOS dependencies on Debian Stretch
===========================================

Here are the steps to install the required dependencies of IKOS on **[Debian 9 (Stretch)](https://wiki.debian.org/DebianStretch)**.

First, make sure your system is up-to-date:

```
$ sudo apt-get update
$ sudo apt-get upgrade
```

Then, run the following commands:

```
$ sudo apt-get install gcc g++ cmake libgmp-dev libboost-dev libboost-filesystem-dev \
    libboost-test-dev python python-pygments libsqlite3-dev libz-dev libedit-dev \
    llvm-4.0 llvm-4.0-dev clang-4.0
```

Now, add the LLVM directory to your `PATH` (consider adding this in your `.bashrc`):

```
$ PATH="/usr/lib/llvm-4.0/bin:$PATH"
```

You are now ready to build IKOS. Go to the section [Build and Install](../README.md#build-and-install) in README.md
