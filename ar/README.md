IKOS Abstract Representation
============================

This folder contains the implementation of the Abstract Representation.

Introduction
------------

The Abstract Representation is an assembly language used internally in IKOS.

Installation
------------

IKOS Abstract Representation is a C++ library. It can be installed independently from the other components.

### Dependencies

To build IKOS AR, you will need the following dependencies:

* A C++ compiler that supports C++14 (gcc >= 4.9.2 or clang >= 3.4)
* CMake >= 2.8.12.2
* GMP >= 4.3.1
* Boost >= 1.55
* IKOS Core

### Build and Install

To build and install IKOS AR, run the following commands in the `ar` directory:

```
$ mkdir build
$ cd build
$ cmake \
    -DCMAKE_INSTALL_PREFIX=/path/to/ar-install-directory \
    -DCORE_ROOT=/path/to/core-install-directory \
    ..
$ make
$ make install
```

### Documentation

To build the documentation, you will need [Doxygen](http://www.doxygen.org).

Then, simply type:

```
$ make doc
$ open doc/html/index.html
```

Overview of the source code
---------------------------

The following illustrates the directory structure of this folder:

```
.
├── doc
│   └── doxygen
├── include
│   └── ikos
│       └── ar
│           ├── format
│           ├── pass
│           ├── semantic
│           ├── support
│           └── verify
└── src
    ├── format
    ├── pass
    ├── semantic
    └── verify
```

#### doc/

Contains Doxygen files.

#### include/

* [include/ikos/ar/format](include/ikos/ar/format) contains definition of formatters for the AR, e.g, a text formatter, a dot formatter, etc.

* [include/ikos/ar/pass](include/ikos/ar/pass) contains definition of passes on the AR.

* [include/ikos/ar/semantic](include/ikos/ar/semantic) contains definition of the AR assembly language, e.g, functions, basic blocks, statements, etc.

* [include/ikos/ar/support](include/ikos/ar/support) contains various helpers, e.g, assertions.

* [include/ikos/ar/verify](include/ikos/ar/verify) contains definition of verifiers for the AR, e.g, a type checker.

#### src/

Contains implementation files, following the structure of `include/ikos/ar`.
