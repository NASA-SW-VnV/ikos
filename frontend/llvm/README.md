IKOS LLVM Frontend
==================

This folder contains implementation of the LLVM frontend for IKOS.

Introduction
------------

The LLVM frontend implements:
* `ikos-pp`, a LLVM bitcode pre-processor for static analysis
* `llvm-to-ar`, a library to translate LLVM bitcode to Abstract Representation (AR)
* `ikos-import`, a translator from LLVM bitcode to AR, used for debugging purpose.

Installation
------------

IKOS LLVM Frontend contains a C++ library and executables. It can be installed independently from the other components.

### Dependencies

To build IKOS LLVM Frontend, you will need the following dependencies:

* A C++ compiler that supports C++14 (gcc >= 4.9.2 or clang >= 3.4)
* CMake >= 3.4.3
* GMP >= 4.3.1
* Boost >= 1.55
* LLVM 14.0.x
* IKOS Core
* IKOS AR

### Build and Install

To build and install IKOS LLVM Frontend, run the following commands in the `frontend/llvm` directory:

```
$ mkdir build
$ cd build
$ cmake \
    -DCMAKE_INSTALL_PREFIX=/path/to/frontend-llvm-install-directory \
    -DLLVM_CONFIG_EXECUTABLE=/path/to/llvm/bin/llvm-config \
    -DCORE_ROOT=/path/to/core-install-directory \
    -DAR_ROOT=/path/to/ar-install-directory \
    ..
$ make
$ make install
```

### Tests

To build and run the tests, simply type:

```
$ make check
```

Running the LLVM Frontend Tools
-------------------------------

### lib/libikos-llvm-to-ar.a

`llvm-to-ar` is a library to translate LLVM bitcode to AR, used by the analyzer and `ikos-import`.

### ikos-pp

`ikos-pp` is a LLVM bitcode pre-processor for static analysis.

It is similar to the LLVM `opt` command, see https://llvm.org/docs/CommandGuide/opt.html

See `ikos-pp -help` for more information.

### ikos-import

`ikos-import` is a translator from LLVM bitcode to AR, used for debugging purpose.

See `ikos-import -help` for more information.

Overview of the source code
---------------------------

The following illustrates the directory structure of this folder:

```
.
├── include
│   └── ikos
│       └── frontend
│           └── llvm
│               └── import
├── src
│   ├── import
│   └── pass
└── test
    └── regression
        ├── import
        │   ├── aggressive_optimization
        │   ├── basic_optimization
        │   └── no_optimization
        └── pass
            ├── lower_cst_expr
            ├── lower_select
            ├── remove_printf_calls
            └── remove_unreachable_blocks
```

#### include/

* [include/ikos/frontend/llvm/import](include/ikos/frontend/llvm/import) contains definition of the translation from LLVM to AR.

* [include/ikos/frontend/llvm/pass.hpp](include/ikos/frontend/llvm/pass.hpp) contains definition of LLVM passes for helping static analysis.

#### src/

* [src/ikos_import.cpp](src/ikos_import.cpp) contains implementation of `ikos-import`.

* [src/ikos_pp.cpp](src/ikos_pp.cpp) contains implementation of `ikos-pp`.

* [src/import](src/import) contains implementation of the translation from LLVM to AR.

* [src/pass](src/pass) contains implementation of LLVM passes for helping static analysis.

#### test/

Contains regression tests.
