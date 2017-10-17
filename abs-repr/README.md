Abstract Representation
=======================

This folder contains the implementation of the ARBOS tool.

Introduction
------------

The ARBOS tool produces a program model from an _AR file_. An _AR file_ is a text file that represents the program model in S-expressions using the Abstract Representation (AR) specification. An _AR file_ is produced by the tools under [../frontends](../frontends).

ARBOS is a plugin framework that reads an _AR file_ and creates the AR program model in memory for registered ARBOS passes to traverse and perform analysis on it.

Related documents include:

* [docs/AR_SPEC.md](docs/AR_SPEC.md) documents the AR specification

Build and Install
-----------------

The ARBOS tool can be independently built. The source project uses cmake to generate build files needed by your build tool.

To build arbos, run the following commands within the abs-repr directory:

```
$ mkdir build
$ cd build
$ cmake -DCMAKE_INSTALL_PREFIX=<install-directory> ..
$ make
$ make install
```

### Example passes

We provide example passes to illustrate how to extend ARBOS by integrating your own passes. The source code of these passes are located under [src/passes/examples](src/passes/examples). To build these example passes, you can simply run the following command in the same `build` directory:

```
$ make example-passes
```

And use the following command to install the example passes:

```
$ make install
```

### Documentation

To build and install our doxygen documentation, run the following command under the same `build` directory:

```
$ make docs
```

and use the following command to instal the documentation:

```
$ make install
```

### Running the regression tests

The regression tests are verified by verifiers implemented as ARBOS passes. These passes are located under [tests/regr-tests/verifier-passes](tests/regr-tests/verifier-passes) and must be built before we can run the regression tests.

To build and install the verifier passes, run the following command under the same `build` directory:

```
$ make verifier-passes
$ make install
```

To run the regression tests, please make sure to add IKOS to your path:

```
$ PATH=/path/to/ikos-install-directory/bin:$PATH
```

Now, simply run:

```
$ make test
```

How to run ARBOS
----------------

The **arbos** command requires users to specify a list of ARBOS passes to run on the AR provided by the input AR file. Suppose we want to run the **Hello World** ARBOS pass on an AR file `file.ar`, then we use the following command under the directory where you've generated `file.ar`:

```
$ arbos -load=/path/to/libhello.dylib -hello < file.ar
```

You should obtain the following output:

```
dlopen successful on libhello.dylib
Loaded ARBOS pass: hello - Demo pass that prints Hello World!
1 pass(es) registered.
Executing pass - hello Demo pass that prints Hello World!
Hello World!
```

What you have done is to execute the **Hello World** ARBOS pass on the AR generated from `file.ar`. This pass simply prints "Hello World!" in the console.

Writing an ARBOS pass
---------------------

The ARBOS example passes are located under [src/passes/examples](src/passes/examples).

If you want to create a new pass, the easiest way is to create it out of a template. The `hello` pass is an obvious choice. Suppose you want to create a pass named `mypass`. Below shows the implementation of the pass:

```c++
#include <iostream>

#include "arbos/semantics/ar.hpp"

using namespace arbos;

namespace {

class MyPass : public Pass {
public:
  MyPass() : Pass("mypass", "My ARBOS pass.") {};
  virtual ~MyPass() {}
  virtual void execute(AR_Node_Ref< AR_Bundle > bundle) {
    std::cout << "This is the entry point for the pass" << std::endl;
  }
};

}

extern "C" Pass* init() {
   return new MyPass();
}
```

Then go under the [examples](abs-repr/src/passes/examples) directory and edit the [CMakeLists.txt](abs-repr/src/passes/examples/CMakeLists.txt) file. Add the following line at the beginning of the file:

```
add_library(mypass SHARED mypass.cpp)
target_link_libraries(mypass arbos-api)
install(TARGETS mypass DESTINATION lib OPTIONAL)
```

And append `mypass` in the list of arguments in `add_custom_target`:

```
add_custom_target(example-passes DEPENDS hello options visitor mypass)
```

Now you can build and installed the example passes using the following command:

```
$ make example-passes
$ make install
```

`mypass` is now built. To try it out, please go under the directory where `file.ar` is located and type:

```
$ arbos -load=libmypass.dylib -mypass < file.ar
```

You should obtain the following output:

```
dlopen successful on libmypass.dylib
Loaded ARBOS pass: mypass - My ARBOS pass.
1 pass(es) registered.
Executing pass - mypass My ARBOS pass.
This is the entry point for the pass
```

Overview of the source code
---------------------------

```
.
├── CMakeLists.txt
├── README.md
├── docs
│   ├── AR_SPEC.md
│   └── doxygen
│       ├── Doxyfile.in
│       ├── DoxygenLayout.xml
│       └── customdoxygen.css
├── include
│   └── arbos
│       ├── common
│       ├── io
│       └── semantics
├── src
│   ├── arbos
│   │   ├── api
│   │   └── arbos.cpp
│   └── passes
│       ├── dumps
│       └── examples
└── tests
    └── regression
        ├── CMakeLists.txt
        ├── tests
        ├── utils
        └── verifier-passes
```

#### docs/

* [docs/AR_SPEC.md](docs/AR_SPEC.md) documents the AR specification.

* [docs/doxygen](docs/doxygen) contains Doxygen files.

#### include/

* [include/arbos/common](include/arbos/common) contains all basic data structures used across the infrastructure. For example, FP representation, big integers, etc.

* [include/arbos/io](include/arbos/io) contains interfaces and adapters for various exchange formats for AR. For example, importing and exporting AR from and to S-expressions.

* [include/arbos/semantics](include/arbos/semantics) contains data structures for defining semantics for the CFG.

#### src/

* [src/arbos/api](src/arbos/api) contains implementation of ARBOS API.

* [src/arbos/arbos.cpp](src/arbos/arbos.cpp) implements the driver of the plugin framework.

* [src/passes/examples](src/passes/examples) contains sample ARBOS passes.

#### tests/

* [tests/regression/tests](tests/regression/tests) contains test programs in C/C++ for verifying the generated AR program models.

* [tests/regression/utils](tests/regression/utils) contains scripts that support the execution of the regression tests.

* [tests/regression/verifier-passes](tests/regression/verifier-passes) contains ARBOS passes that verify the generated AR program models.
