IKOS Core
=========

This folder contains implementation of the theory of Abstract Interpretation.

CONTENTS OF THIS FILE
---------------------

* Introduction
* Overview of Source Code

INTRODUCTION
------------

The IKOS core is a C++ library designed to facilitate the development of sound static analyzers based on Abstract Interpretation. Specialization of a static analyzer for an application or family of applications is critical for achieving both precision and scalability. Developing such an analyzer is arduous and requires significant expertise in Abstract Interpretation. The IKOS core library provides a generic and efficient implementation of state-of-the-art Abstract Interpretation data structures and algorithms, such as control-flow graphs, fixpoint iterators, numerical abstract domains, etc. The IKOS code is independent of a particular programming language. In order to build an effective static analyzer, one has to use the IKOS core building blocks in combination with a front-end for a particular language.

OVERVIEW OF THE SOURCE CODE
---------------------------

The following illustrates the content of this directory:

```
.
├── CMakeLists.txt
├── README.md
├── docs
│   └── doxygen
│       └── Doxyfile.in
├── include
│   └── ikos
│       ├── algorithms
│       ├── common
│       ├── domains
│       ├── examples
│       ├── iterators
│       └── semantics
└── tests
    └── unit
```

#### include/

`include/ikos/algorithms` contains implementation of supporting algorithms, e.g., Patricia Trees, union-find, linear solvers, etc.

`include/ikos/domains` contains implementation of abstract domains, e.g., the interval, congruence, difference-bound matrices, octagon domains, etc.

`include/ikos/iterators` contains implementation of fixpoint iterators.

`include/ikos/semantics` contains implementation of the control-flow graph API.

`include/ikos/common` contains common utilities.

`include/ikos/examples` contains the muZQ example that demonstrates how to use the IKOS core library. muZQ is a micro language for semantic modeling over integer and rational numbers.

#### docs/

Contains Doxygen files.

#### tests/

Contains unit tests.
