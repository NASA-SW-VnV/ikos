Install IKOS on CentOS 6.10
===========================

**NOTE: These instructions are for IKOS 3.0 with LLVM 9 and are not actively maintained. Please see the main [README.md](../../../README.md)**

Here are the steps to install IKOS and its dependencies on **[CentOS 6.10](https://www.centos.org/)**

IKOS requires certain versions of cmake, apron, boost and llvm that are newer than the ones available on the CentOS Yum package manager. Hence the following describes the steps to bootstrap these dependencies on CentOS.

First, make sure your system is up-to-date:

```
$ sudo yum update
```

Install the following packages using yum:

```
$ sudo yum install patch bzip2 xz zlib-devel libedit-devel make m4 sqlite-devel tbb-devel
```

We will use [CentOS Sofware Collections (CSL)](https://wiki.centos.org/AdditionalResources/Repositories/SCL) to get a more recent version of gcc and python. Enable the CSL by running:

```
$ sudo yum install centos-release-scl
```

Now, you can install the following packages:

```
$ sudo yum install devtoolset-8-gcc devtoolset-8-gcc-c++ python27
```

You need to update your `PATH` and `LD_LIBRARY_PATH` to use devtoolset-8 and python27. Use the following commands (consider adding this in your `.bashrc`):

```
$ PATH="/opt/rh/python27/root/usr/bin:/opt/rh/devtoolset-8/root/usr/bin:$PATH"
$ export LD_LIBRARY_PATH="/opt/rh/python27/root/usr/lib64:/opt/rh/devtoolset-8/root/usr/lib64:/opt/rh/devtoolset-8/root/usr/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
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
├── gmp-6.1.2
│   ├── include
│   ├── lib
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
├── mpfr-4.0.2
│   ├── include
│   ├── lib
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

You are now ready to use IKOS. Go to the section [How to Run IKOS](../../../README.md#how-to-run-ikos) in README.md
