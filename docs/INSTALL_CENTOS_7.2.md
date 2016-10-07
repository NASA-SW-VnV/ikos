INSTALL IKOS ON CENTOS 7.2
==========================

Here are the steps to install IKOS and its dependencies on **CentOS 7.2**

IKOS requires certain versions of cmake, boost and llvm that are newer than the ones available on the CentOS Yum package manager. Hence the following describes the steps to bootstrap these dependencies on CentOS.

First, make sure your system is up-to-date:

```
$ sudo yum update
```

Install the following packages using yum:

```
$ sudo yum install which patch bzip2 xz zlib-devel libedit-devel sqlite-devel gmp-devel make
```

We will use [CentOS Sofware Collections (CSL)](https://wiki.centos.org/AdditionalResources/Repositories/SCL) to get a more recent version of gcc. Enable the CSL by running:

```
$ sudo yum install centos-release-scl
```

Now, you can install the following packages:

```
$ sudo yum install devtoolset-4-gcc devtoolset-4-gcc-c++
```

You need to update your `PATH` and `LD_LIBRARY_PATH` to use devtoolset-4. Use the following commands (consider adding this in your `.bashrc`):

```
$ PATH="/opt/rh/devtoolset-4/root/usr/bin:$PATH"
$ export LD_LIBRARY_PATH="/opt/rh/devtoolset-4/root/usr/lib64:/opt/rh/devtoolset-4/root/usr/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
```

The next step will use the bootstrap script to build and install cmake, boost, llvm and IKOS.
Here, we will use `/path/to/ikos-install` as the installation directory and `/path/to/ikos-build` as the build directory. Replace it with the location where you want to put IKOS and its dependencies (for instance, `~/ikos-install` and `~/ikos-build`).

In IKOS root directory, run:

```
$ cd scripts
$ ./bootstrap --prefix=/path/to/ikos-install --builddir=/path/to/ikos-build
```

You can also specify the number of CPU you want to use with `--jobs=N`. By default, it will use **all available CPU**.

Note: Try to avoid whitespaces in the installation path, as it might fail.

This script will also build and install IKOS for you.

After installation, the install directory will contain the following structure:

```
.
├── activate-full
├── activate-minimal
├── boost-1.61.0
│   ├── include
│   └── lib
├── cmake-3.6.1
│   ├── bin
│   ├── doc
│   └── share
├── ikos-1.2
│   ├── bin
│   ├── include
│   └── lib
└── llvm-3.7.1
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

For more information about the **bootstrap** script and how to run the tests, see `docs/INSTALL_ROOTLESS.md`.

You are now ready to use IKOS. Go to the section `How to Run IKOS` of the `README.md`.
