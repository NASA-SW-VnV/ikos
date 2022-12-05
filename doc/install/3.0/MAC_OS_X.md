Install IKOS on Mac OS X
========================

**NOTE: These instructions are for IKOS 3.0 with LLVM 9 and are not actively maintained. Please see the main [README.md](../../../README.md)**

Here are the steps to install IKOS on **[MAC OS X](https://www.apple.com/macos/high-sierra/)** using **[Homebrew](https://brew.sh/)**.

First, make sure your system is up-to-date:

```
$ brew upgrade
```

Now, install IKOS:

```
$ brew install nasa-sw-vnv/core/ikos
```

You are now ready to use IKOS. Go to the section [How to Run IKOS](../../../README.md#how-to-run-ikos) in README.md

Build IKOS from source on Mac OS X
==================================

Here are the steps to install the required dependencies to build IKOS from source on **[MAC OS X](https://www.apple.com/macos/high-sierra/)** using **[Homebrew](https://brew.sh/)**.

First, make sure your system is up-to-date:

```
$ brew upgrade
```

Now, install the following packages:

```
$ brew install cmake gmp boost tbb llvm@14
```

When running cmake to build IKOS, you will need to define `LLVM_CONFIG_EXECUTABLE`:

```
$ cmake \
    -DCMAKE_INSTALL_PREFIX="/path/to/ikos-install-directory" \
    -DLLVM_CONFIG_EXECUTABLE="$(brew --prefix)/opt/llvm@14/bin/llvm-config" \
    -DSQLITE3_ROOT="$(brew --prefix)/opt/sqlite3"
    ..
```

You are now ready to build IKOS. Go to the section [Build and Install](../../../README.md#build-and-install) in README.md
