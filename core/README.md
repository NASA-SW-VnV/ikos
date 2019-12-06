IKOS Core
=========

This folder contains implementation of the theory of Abstract Interpretation.

Introduction
------------

The IKOS Core is a C++ library designed to facilitate the development of sound static analyzers based on Abstract Interpretation. Specialization of a static analyzer for an application or family of applications is critical for achieving both precision and scalability. Developing such an analyzer is arduous and requires significant expertise in Abstract Interpretation. The IKOS core library provides a generic and efficient implementation of state-of-the-art Abstract Interpretation data structures and algorithms, such as control-flow graphs, fixpoint iterators, numerical abstract domains, etc. The IKOS code is independent of a particular programming language. In order to build an effective static analyzer, one has to use the IKOS core building blocks in combination with a front-end for a particular language.

Installation
------------

IKOS Core is a header-only C++ library. It can be installed independently from the other components.

### Dependencies

To use IKOS Core, you will need the following dependencies:

* A C++ compiler that supports C++14 (gcc >= 4.9.2 or clang >= 3.4)
* CMake >= 2.8.12.2
* GMP >= 4.3.1
* Boost >= 1.55
* (Optional) TBB >= 2
* (Optional) APRON >= 0.9.10

### Install

To install IKOS Core, run the following commands in the `core` directory:

```
$ mkdir build
$ cd build
$ cmake -DCMAKE_INSTALL_PREFIX=/path/to/core-install-directory ..
$ make install
```

### Tests

To build and run the tests, simply type:

```
$ make check
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
│       └── core
│           ├── adt
│           │   └── patricia_tree
│           ├── domain
│           │   ├── exception
│           │   ├── lifetime
│           │   ├── machine_int
│           │   ├── memory
│           │   │   └── value
│           │   ├── nullity
│           │   ├── numeric
│           │   ├── pointer
│           │   └── uninitialized
│           ├── example
│           │   └── machine_int
│           ├── fixpoint
│           ├── number
│           ├── semantic
│           │   ├── machine_int
│           │   ├── memory
│           │   └── pointer
│           ├── support
│           └── value
│               ├── machine_int
│               ├── numeric
│               └── pointer
└── test
    └── unit
        ├── adt
        │   └── patricia_tree
        ├── domain
        │   ├── machine_int
        │   ├── nullity
        │   ├── numeric
        │   │   └── apron
        │   ├── pointer
        │   └── uninitialized
        ├── example
        ├── number
        └── value
            ├── machine_int
            └── numeric
```

#### doc/

Contains Doxygen files.

#### include/

* [include/ikos/core/adt](include/ikos/core/adt) contains implementation of Abstract Data Types, e.g., patricia trees.

* [include/ikos/core/domain](include/ikos/core/domain) contains implementation of abstract domains.

* [include/ikos/core/domain/machine_int](include/ikos/core/domain/machine_int) contains implementation of machine integer abstract domains.

* [include/ikos/core/domain/memory](include/ikos/core/domain/memory) contains implementation of memory abstract domains.

* [include/ikos/core/domain/numeric](include/ikos/core/domain/numeric) contains implementation of numerical abstract domains, e.g., the interval, congruence, difference-bound matrices, octagon domains, etc.

* [include/ikos/core/domain/pointer](include/ikos/core/domain/pointer) contains implementation of pointer abstract domains and pointer constraints solvers.

* [include/ikos/core/example](include/ikos/core/example) contains usage examples, e.g., muZQ: is a micro language for semantic modeling over integer and rational numbers.

* [include/ikos/core/fixpoint](include/ikos/core/fixpoint) contains implementation of fixpoint iterators.

* [include/ikos/core/number](include/ikos/core/number) contains implementation of numbers, e.g, integers, rationals and machine integers.

* [include/ikos/core/semantic](include/ikos/core/semantic) contains definition of traits, e.g., the control flow graph traits.

* [include/ikos/core/support](include/ikos/core/support) contains various helpers, e.g, assertions.

* [include/ikos/core/value](include/ikos/core/value) contains implementation of abstract values, e.g, intervals, congruences, etc.

#### test/

Contains unit tests.
