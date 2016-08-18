INSTALL IKOS DEPENDENCIES ON UBUNTU 16.04
=========================================

Here are the steps to install the required dependencies of IKOS on **Debian 16.04 (Xenial)**.

First, make sure your system is up-to-date:

```
$ sudo apt-get update
$ sudo apt-get upgrade
```

Then, run the following commands:

```
$ sudo apt-get install cmake libgmp-dev libsqlite3-dev libz-dev libedit-dev libboost-dev libboost-program-options-dev libboost-filesystem-dev gcc g++ llvm-3.7 clang-3.7
```

Now, add the LLVM directory to your `PATH` (consider adding this in your `.bashrc`):

```
$ PATH="/usr/lib/llvm-3.7/bin:$PATH"
```

Unfortunately, there is a problem in the **llvm-3.7** ubuntu package, you will need to provide an extra argument to cmake when you build IKOS in the next section:

```
$ cmake -DCMAKE_INSTALL_PREFIX=/path/to/ikos-install-directory -DCMAKE_MODULE_PATH=/usr/share/llvm-3.7/cmake ..
```

You are now ready to build IKOS. Go to the next section of the `README.md`.
