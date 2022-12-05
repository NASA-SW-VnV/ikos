Install IKOS dependencies on Ubuntu 19.04
=========================================

**NOTE: These instructions are for IKOS 3.0 with LLVM 9 and are not actively maintained. Please see the main [README.md](../../../README.md)**

Here are the steps to install the required dependencies of IKOS on **[Ubuntu 19.04 (Disco Dingo)](http://releases.ubuntu.com/19.04/)**.

First, make sure your system is up-to-date:

```
$ sudo apt-get update
$ sudo apt-get upgrade
```

Now, you will need to add the LLVM repository to your apt `sources.list`:

```
$ echo "deb http://apt.llvm.org/disco/ llvm-toolchain-disco-9 main" | sudo tee -a /etc/apt/sources.list
```

You also need to trust the LLVM repository key:

```
$ wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
```

Then, run the following commands:

```
$ sudo apt-get update
$ sudo apt-get install gcc g++ cmake libgmp-dev libboost-dev libboost-filesystem-dev \
    libboost-thread-dev libboost-test-dev python python-pygments libsqlite3-dev libtbb-dev \
    libz-dev libedit-dev llvm-9 llvm-9-dev llvm-9-tools clang-9
```

When running cmake to build IKOS, you will need to define `LLVM_CONFIG_EXECUTABLE`:

```
$ cmake \
    -DCMAKE_INSTALL_PREFIX="/path/to/ikos-install-directory" \
    -DLLVM_CONFIG_EXECUTABLE="/usr/lib/llvm-9/bin/llvm-config" \
    ..
```

You are now ready to build IKOS. Go to the section [Build and Install](../../../README.md#build-and-install) in README.md
