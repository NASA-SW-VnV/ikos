INSTALL IKOS DEPENDENCIES ON UBUNTU 12.04
=========================================

Here are the steps to install the required dependencies of IKOS on **Debian 12.04 (Precise)**.

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
$ sudo add-apt-repository -y "deb http://apt.llvm.org/precise/ llvm-toolchain-precise-3.7 main"
```

Note: They provide, line by line: cmake 3.2, gcc 4.9, boost 1.55.0 and llvm 3.7

Then, run the following commands:

```
$ sudo apt-get update
$ sudo apt-get install cmake libgmp-dev libsqlite3-dev libz-dev libedit-dev libboost1.55-dev libboost-program-options1.55-dev libboost-filesystem1.55-dev gcc-4.9 g++-4.9 llvm-3.7 clang-3.7
```

Now, add the LLVM directory to your `PATH` (consider adding this in your `.bashrc`):

```
$ PATH="/usr/lib/llvm-3.7/bin:$PATH"
```

Define `CC` and `CXX` to use gcc 4.9 instead of the default compiler:

```
$ export CC=gcc-4.9
$ export CXX=g++-4.9
```

Unfortunately, there is a problem in the **llvm-3.7** ubuntu package, you will need to provide an extra argument to cmake when you build IKOS in the next section:

```
$ cmake -DCMAKE_INSTALL_PREFIX=/path/to/ikos-install-directory -DCMAKE_MODULE_PATH=/usr/share/llvm-3.7/cmake ..
```

You are now ready to build IKOS. Go to the next section of the `README.md`.
