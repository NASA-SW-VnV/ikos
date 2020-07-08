Install IKOS on CentOS 8
========================

Here are the steps to install IKOS and its dependencies on **[CentOS 8](https://www.centos.org/)**

IKOS requires certain versions of cmake, apron, boost and llvm that are newer than the ones available on the CentOS Yum package manager. Hence the following describes the steps to bootstrap these dependencies on CentOS.

First, make sure your system is up-to-date:

```
$ sudo dnf update
```

Install the development tools package using dnf:

```
sudo dnf group install "Development Tools"
```

Install other required tools using dnf:

```
$ sudo dnf install which patch bzip2 xz make m4 python3 python-pygments
```

Install required libraries which the bootstrap script fails to build

```
$ sudo dnf install ncurses-devel
```

In the next step, we will use the bootstrap script to build and install cmake, apron, boost, llvm and IKOS.
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
├── boost-1.70.0
│   ├── include
│   └── lib
├── cmake-3.15.2
│   ├── bin
│   ├── doc
│   └── share
├── ikos-3.0
│   ├── bin
│   ├── include
│   ├── lib
│   └── share
├── llvm-9.0.0
│   ├── bin
│   ├── include
│   ├── lib
│   ├── libexec
│   └── share
└── ppl-1.2
    ├── bin
    ├── include
    ├── lib
    └── share
```

During its execution, the **bootstrap** script creates a special environment with all the required dependencies for IKOS. To enter that environment, simply run the following command (consider adding this in your `.bashrc`):

```
source /path/to/ikos-install/activate-minimal
```

For short, it adds the necessary directories to your `PATH` and your `LD_LIBRARY_PATH`.

For more information about the **bootstrap** script and how to run the tests, see [ROOTLESS.md](ROOTLESS.md).

You are now ready to use IKOS. Go to the section [How to Run IKOS](../../README.md#how-to-run-ikos) in README.md
