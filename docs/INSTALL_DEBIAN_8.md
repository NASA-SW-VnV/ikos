INSTALL IKOS DEPENDENCIES ON DEBIAN JESSIE
==========================================

Here are the steps to install the required dependencies of IKOS on **Debian 8 (Jessie)**.

First, make sure your system is up-to-date:

```
$ sudo apt-get update
$ sudo apt-get upgrade
```

Now, you will need to add the LLVM repositories to your apt `sources.list`:

```
$ echo "deb http://apt.llvm.org/jessie/ llvm-toolchain-jessie-3.7 main" | sudo tee -a /etc/apt/sources.list
```

Then, run the following commands:

```
$ sudo apt-get update
$ sudo apt-get install cmake libgmp-dev libsqlite3-dev libz-dev libedit-dev libboost-dev libboost-program-options-dev libboost-filesystem-dev llvm-3.7 clang-3.7
```

Now, add the LLVM directory to your `PATH` (consider adding this in your `.bashrc`):

```
$ PATH="/usr/lib/llvm-3.7/bin:$PATH"
```

Unfortunately, there is a problem in the **llvm-3.7** debian package, you will need to provide an extra argument to cmake when you build IKOS in the next section:

```
$ cmake -DCMAKE_INSTALL_PREFIX=/path/to/ikos-install-directory -DCMAKE_MODULE_PATH=/usr/share/llvm-3.7/cmake ..
```

You are now ready to build IKOS. Go to the next section of the `README.md`.
