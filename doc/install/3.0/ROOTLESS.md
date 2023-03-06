Install IKOS without root access
================================

**NOTE: These instructions are for IKOS 3.0 with LLVM 9 and are not actively maintained. Please see the main [README.md](../../../README.md)**

Here are instructions to install IKOS and its dependencies without sudo or root access.

IKOS provides a **bootstrap** script that is able to detect your current configuration. It will check for all IKOS dependencies. All missing dependencies are automatically downloaded, built and installed.

Minimal requirements
--------------------

The **bootstrap** script should work on any UNIX environment. It has been tested under Arch Linux, CentOS, Debian, Fedora, Mac OS X, Red Hat and Ubuntu.

The script requires some binary utilities to work properly.

It assumes you have the following commands: `bash`, `basename`, `dirname`, `mkdir`, `touch`, `sed` and `date`. If you don't, you should install them manually.

The script automatically checks for all the other required utilities (such as `uname`, `which`, `patch`, etc.). Just install the missing dependencies manually whenever the script tells you a tool is missing.

You will need a **C/C++ compiler**. The version of the compiler does not matter. If you have an old compiler that does not support C++ 14, the script will automatically download, build and install **gcc 9.2.0**.

You will also need a tool to download tarballs, such as **curl** or **wget**.

Disk space and time
-------------------

The script might take a lot of disk space and a lot of time to complete.

If your compiler does not support C++ 14 (ie. gcc < 4.9.2 or clang < 3.4), you will need **at least 10 Gb**. The script might run for a **few hours**, depending on your number of CPU.

Otherwise, **3 to 5 Gb** should be enough, and it should be done in at most **1 to 2 hours**.

How to build IKOS
-----------------

First, move in the `script` directory:

```
$ cd script
```

We will use the **bootstrap** script. Run `./bootstrap -h` to get all available options.

Here, we will use `/path/to/ikos-install` as the installation directory and `/path/to/ikos-build` as the build directory. Replace it with the location where you want to put IKOS and its dependencies (for instance, `~/ikos-install` and `~/ikos-build`).

To build IKOS and its dependencies, run:

```
$ ./bootstrap --prefix=/path/to/ikos-install --builddir=/path/to/ikos-build
```

You can also specify the number of CPU you want to use with `--jobs=N`. By default, it will use **all available CPU**.

Note: Try to avoid whitespaces in the installation path, as it might fail.

How to use IKOS after installation
----------------------------------

After installation, the install directory will contain the following structure:

```
.
├── activate-full
├── activate-minimal
├── [...]
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

Note that after installation, the build directory is only necessary if you want to run the tests again.

During its execution, the **bootstrap** script creates a special environment with all the required dependencies for IKOS. To enter that environment, simply run the following command (consider adding this in your `.bashrc`):

```
$ source /path/to/ikos-install/activate-minimal
```

For short, it adds the necessary directories to your `PATH` and your `[DY]LD_LIBRARY_PATH`.

The difference between `activate-minimal` and `activate-full` is that `activate-minimal` only adds the necessary directories to run IKOS. `activate-full` also adds the directories required to build IKOS (for instance, cmake).

You are now ready to use IKOS. Go to the section [How to Run IKOS](../../../README.md#how-to-run-ikos) in README.md

How to run the tests
--------------------

The bootstrap script automatically runs the tests.

If you want to run the tests again, you first need to enter the build environment using:

```
$ source /path/to/ikos-install/activate-full
```

To run the tests:

```
$ cd /path/to/ikos-build/ikos-3.0
$ make check
```

Note that you can safely remove the build directory if you don't want to run the tests anymore.
