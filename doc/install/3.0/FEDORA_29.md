Install IKOS dependencies on Fedora 29
======================================

**NOTE: These instructions are for IKOS 3.0 with LLVM 9 and are not actively maintained. Please see the main [README.md](../../../README.md)**

Here are the steps to install IKOS and its dependencies on **[Fedora 29](https://getfedora.org/)**.

IKOS requires a specific version of llvm and clang that are not available on the Fedora package manager. Hence the following describes the steps to bootstrap these dependencies.

First, make sure your system is up-to-date:

```
$ sudo dnf update
```

Now, install the following packages:

```
$ sudo dnf install which findutils patch bzip2 xz zlib-devel libedit-devel \
    gcc gcc-c++ make cmake m4 gmp-devel mpfr-devel ppl-devel boost-devel \
    python python-pygments sqlite-devel tbb-devel
```

In the next step, we will use the bootstrap script to build and install llvm, clang and IKOS.
Here, we will use `/path/to/ikos-install` as the installation directory and `/path/to/ikos-build` as the build directory. Replace it with the location where you want to put IKOS and its dependencies (for instance, `~/ikos-install` and `~/ikos-build`).

In IKOS root directory, run:

```
$ cd script
$ ./bootstrap --prefix=/path/to/ikos-install --builddir=/path/to/ikos-build
```

You can also specify the number of CPU you want to use with `--jobs=N`. By default, it will use **all available CPU**.

Note: Try to avoid whitespaces in the installation path, as it might fail.

This script will also build, install and run the tests of IKOS for you.

After installation, the install directory will contain the following structure:

```
.
├── activate-full
├── activate-minimal
├── apron-0.9.10
│   ├── bin
│   ├── include
│   └── lib
├── ikos-3.0
│   ├── bin
│   ├── include
│   ├── lib
│   └── share
└── llvm-9.0.0
    ├── bin
    ├── include
    ├── lib
    ├── libexec
    └── share
```

During its execution, the **bootstrap** script creates a special environment with all the required dependencies for IKOS. To enter that environment, simply run the following command (consider adding this in your `.bashrc`):

```
source /path/to/ikos-install/activate-minimal
```

For short, it adds the necessary directories to your `PATH` and your `LD_LIBRARY_PATH`.

For more information about the **bootstrap** script and how to run the tests, see [ROOTLESS.md](ROOTLESS.md).

You are now ready to use IKOS. Go to the section [How to Run IKOS](../../../README.md#how-to-run-ikos) in README.md
