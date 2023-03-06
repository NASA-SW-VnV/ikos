Install IKOS dependencies on Windows
====================================

**NOTE: These instructions are for IKOS 3.0 with LLVM 9 and are not actively maintained. Please see the main [README.md](../../../README.md)**

Here are the steps to install the required dependencies of IKOS on **[Windows](https://www.microsoft.com/en-us/windows)** using **[MSYS2](https://www.msys2.org/)**.

First, install **MSYS2** by following the instructions at https://www.msys2.org/

Then, use the "MSYS2 MinGW 64-bit" terminal from the Start menu.

Make sure your system is up-to-date:

```
$ pacman -Syu
```

Now, install the following packages:

```
$ pacman -S \
    mingw-w64-x86_64-gcc \
    make \
    mingw-w64-x86_64-cmake \
    mingw-w64-x86_64-gmp \
    mingw-w64-x86_64-boost \
    mingw-w64-x86_64-python3 \
    mingw-w64-x86_64-python3-pygments \
    mingw-w64-x86_64-sqlite3 \
    mingw-w64-x86_64-intel-tbb \
    mingw-w64-x86_64-llvm \
    mingw-w64-x86_64-clang \
    mingw-w64-x86_64-polly
```

When running cmake to build IKOS, you will need to provide the following parameters:

```
$ cmake \
    -DCMAKE_INSTALL_PREFIX="/path/to/ikos-install-directory" \
    -G "MSYS Makefiles" \
    ..
```

You are now ready to build IKOS. Go to the section [Build and Install](../../../README.md#build-and-install) in README.md
