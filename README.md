IKOS 1.3
========

IKOS (Inference Kernel for Open Static Analyzers) is a static analyzer for C/C++ based on the theory of Abstract Interpretation.

Introduction
------------

IKOS started as a C++ library designed to facilitate the development of sound static analyzers based on [Abstract Interpretation](https://www.di.ens.fr/~cousot/AI/IntroAbsInt.html). Specialization of a static analyzer for an application or family of applications is critical for achieving both precision and scalability. Developing such an analyzer is arduous and requires significant expertise in Abstract Interpretation.

IKOS provides a generic and efficient implementation of state-of-the-art Abstract Interpretation data structures and algorithms, such as control-flow graphs, fixpoint iterators, numerical abstract domains, etc. IKOS is independent of a particular programming language.

IKOS now also provides a C/C++ analyzer based on [LLVM](https://llvm.org/). It implements scalable analyses for detecting buffer-overflows, divisions by zero, null pointer dereferences, uninitialized variables and unaligned pointer dereferences in C and C++ programs.

License
-------

IKOS has been released under the NASA Open Source Agreement version 1.3, see [LICENSE.pdf](LICENSE.pdf)

Contact
-------

ikos@lists.nasa.gov

Release notes
-------------

See [RELEASE_NOTES.md](RELEASE_NOTES.md)

Troubleshooting
---------------

See [TROUBLESHOOTING.md](TROUBLESHOOTING.md)

Build and Install
-----------------

IKOS analyzes programs transformed into the Abstract Representation (AR), an intermediate representation that represents the control-flow graph of the program. The IKOS distribution provides a compiler frontend that transforms C/C++ programs into the AR using the LLVM compiler framework.

The next section illustrates how to install the required dependencies.

### Dependencies

To build and run the analyzer, you will need the following dependencies:

* CMake >= 2.8.12.2
* GMP >= 4.3.1
* Boost >= 1.55
* Python 2 >= 2.7.3 or Python 3 >= 3.3
* SQLite >= 3.6.20
* LLVM and Clang 4.0.x
* A C++ compiler that supports C++14 (gcc >= 4.9.2 or clang >= 3.4)

Note: You will need CMake >= 3.4.3 if you build LLVM from source

Most of them can be installed using your package manager.

Installation instructions for Archlinux, CentOS, Debian, Fedora, Mac OS X, Red Hat and Ubuntu are available in the [docs](docs) directory. These instructions assume you have sudo or root access. If you don't, please follow the instructions in [docs/INSTALL_ROOTLESS.md](docs/INSTALL_ROOTLESS.md).

Once you have all the required dependencies, move to the next section.

### Build and Install IKOS

Now that you have all the dependencies on your system, you can build and install IKOS.

As you open the IKOS distribution, you shall see the following content:

```
.
├── CMakeLists.txt
├── LICENSE.pdf
├── README.md
├── RELEASE_NOTES.md
├── TROUBLESHOOTING.md
├── abs-repr
├── analyzer
├── cmake
├── core
├── docs
├── frontends
└── scripts
```

We use CMake to build IKOS. You will need to specify an installation directory that will contain all the binaries, libraries and headers after installation. If you do not specify this directory, CMake will install everything under `install` in the root directory of the distribution. In the following steps, we will install IKOS under `/path/to/ikos-install-directory`. After installation, it will contain the following structure:

```
.
├── include
├── bin
└── lib
```

Here are the steps to build and install IKOS:

```
$ mkdir build
$ cd build
$ cmake -DCMAKE_INSTALL_PREFIX=/path/to/ikos-install-directory ..
$ make
$ make install
```

### Running the tests

To run the tests, we will need the full installation of IKOS. In addition, we will also need to build and install the verifier passes:

```
$ make verifier-passes
$ make install
```

Now, you can run the tests:

```
$ PATH=/path/to/ikos-install-directory/bin:$PATH
$ make test
```

How to run IKOS
---------------

Suppose we want to analyze the following C program in a file, called *loop.c*:

```c
 1: #include <stdio.h>
 2: int a[10];
 3: int main(int argc, char *argv[]) {
 4:     size_t i = 0;
 5:     for (;i < 10; i++) {
 6:         a[i] = i;
 7:     }
 8:     a[i] = i;
 9:     printf("%i", a[i]);
10: }
```

Using the following commands, you can run IKOS to detect buffer overflow defects directly against the C program:

```
$ PATH=/path/to/ikos-install-directory/bin:$PATH
$ ikos loop.c --export
```

Then you shall see the following output. IKOS reports two occurrences of buffer overflow at line 8 and 9.

```
dlopen successful on /path/to/ikos-install-directory/lib/libpointer-shift-opt.dylib
Loaded ARBOS pass: ps-opt - Optimize pointer shift statements
dlopen successful on /path/to/ikos-install-directory/lib/libbranching-opt.dylib
Loaded ARBOS pass: branching-opt - Optimize the Control Flow Graph
dlopen successful on /path/to/ikos-install-directory/lib/libinline-init-gv.dylib
Loaded ARBOS pass: inline-init-gv - Inline initialization of global variables in main
dlopen successful on /path/to/ikos-install-directory/lib/libunify-exit-nodes.dylib
Loaded ARBOS pass: unify-exit-nodes - Unify exit nodes
dlopen successful on /path/to/ikos-install-directory/lib/libanalyzer.dylib
Loaded ARBOS pass: analyzer - Analyzer pass
5 pass(es) registered.
Executing pass - ps-opt Optimize pointer shift statements
Executing pass - branching-opt Optimize the Control Flow Graph
Executing pass - inline-init-gv Inline initialization of global variables in main
Executing pass - unify-exit-nodes Unify exit nodes
Executing pass - analyzer Analyzer pass
Running liveness variable analysis ...
Running function pointer analysis ...
** Generating pointer constraints ...
** Solving pointer constraints ...
Running pointer analysis ...
** Computing intra-procedural numerical invariants ...
** Generating pointer constraints ...
** Solving pointer constraints ...
Running value analysis ...
*** Analyzing entry point: main
*** Analyzing function: __ikos_init_globals
*** Writing results for entry point: main

# Time stats:
arbos     : 0.117 sec
clang     : 0.023 sec
ikos-pp   : 0.006 sec
llvm-to-ar: 0.017 sec

# Summary:
Total number of checks                : 30
Total number of unreachable checks    : 0
Total number of safe checks           : 28
Total number of definite unsafe checks: 2
Total number of warnings              : 0

The program is definitely UNSAFE

# Results
loop.c: In function 'main':
loop.c:8:10: error: buffer overflow, trying to access index 10
    a[i] = i;
         ^
loop.c: In function 'main':
loop.c:9:18: error: buffer overflow, trying to access index 10
    printf("%i", a[i]);
                 ^
```

The ikos command takes a source file (`.c`, `.cpp`) or a LLVM bitcode file (`.bc`) as input, analyzes it to find undefined behaviors (such as buffer overflows), creates a result database `output.db` in the current working directory, prints a summary and exits.

To see the analysis report within your terminal, you shall use the `--export` option. This argument generates a report using the result database, and prints it in a specific format (by default, a gcc-style format).

You can either provide the `--export` option directly in the ikos command, or as a second step using the ikos-render command, taking the result database:

```
$ ikos-render output.db --export
```

To analyze a large program, you shall run the `ikos` command first to create the result database `output.db` and then generate a report using the `ikos-render` command. This way, you can generate reports in different formats and adjust the verbosity to your needs, without running the analysis again.

Further information:
* [Analyses options](analyzer/README.md#analyses-options)
* [Export options](analyzer/README.md#export-options)
* [Export formats](analyzer/README.md#export-formats)
* [Running IKOS on a whole C/C++ project](analyzer/README.md#running-ikos-on-a-whole-cc-project)

Current and Past contributors
-----------------------------

* Maxime Arthaud
* Guillaume Brat
* Clément Decoodt
* Jorge Navas
* Nija Shi
* Sarah Thompson
* Arnaud Venet
* Alexandre Wimmers

Publications
------------

* Guillaume Brat, Jorge Navas, Nija Shi and Arnaud Venet. **IKOS: a Framework for Static Analysis based on Abstract Interpretation.** In _Proceedings of the International Conference on Software Engineering and Formal Methods (SEFM 2014)_, Grenoble, France ([PDF](http://ti.arc.nasa.gov/publications/16610/download/)).

* Arnaud Venet. **The Gauge Domain: Scalable Analysis of Linear Inequality Invariants.** In _Proceedings of Computer Aided Verification (CAV 2012)_, Berkeley, California, USA 2012. Lecture Notes in Computer Science, pages 139-154, volume 7358, Springer 2012 ([PDF](http://ti.arc.nasa.gov/publications/4767/download/)).

Overview of the source code
---------------------------

The following illustrates the content of the root directory:

```
.
├── CMakeLists.txt
├── LICENSE.pdf
├── README.md
├── RELEASE_NOTES.md
├── TROUBLESHOOTING.md
├── abs-repr
├── analyzer
├── cmake
├── core
├── docs
├── frontends
└── scripts
```


* [CMakeLists.txt](CMakeLists.txt) is the root CMake file.

* [LICENSE.pdf](LICENSE.pdf) contains the NOSA 1.3 license

* [RELEASE_NOTES.md](RELEASE_NOTES.md) contains the release notes for the latest versions.

* [TROUBLESHOOTING.md](TROUBLESHOOTING.md) contains solution for common issues with IKOS.

* [cmake](cmake) contains CMake files to search for related software libraries.

* [core](core) contains the implementation of the theory of Abstract Interpretation, which includes the abstract domains, the fixpoint iterator, and various algorithms that support the implementation. More information can be found at [core/README.md](core/README.md).

* [abs-repr](abs-repr) contains the implementation of the ARBOS, a plugin framework that parses the AR and executes AR passes that perform various analysis. More information can be found at [abs-repr/README.md](abs-repr/README.md).

* [analyzer](analyzer) contains the implementation of various analyses for specific defect detections. These analyses are implemented as AR passes and use the fixpoint iterator and abstract domains to perform analysis. More information can be found at [analyzer/README.md](analyzer/README.md).

* [frontends](frontends) contains implementation of various frontend compilers to translate programs into AR. Currently we only support the LLVM frontend. More information can be found at [frontends/llvm/README.md](frontends/llvm/README.md).

* [scripts](scripts) contains the [bootstrap](scripts/bootstrap) script for a rootless installation.

* [docs](docs) contains installation instructions for specific operating systems.
