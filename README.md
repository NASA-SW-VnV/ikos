IKOS v.1.2
==========

LICENSE
-------

IKOS has been released under the NASA Open Source Agreement version 1.3.

CONTACT
-------

ikos@lists.nasa.gov

CONTENTS OF THIS FILE
---------------------

* Introduction
* Build and Install
* How to Run IKOS
* Troubleshooting
* Overview of the Source Code

INTRODUCTION
------------

IKOS is a static analyzer based on the theory of Abstract Interpretation. You can find the release notes for this version in `RELEASE_NOTES.md`.

BUILD AND INSTALL
-----------------

IKOS analyzes programs transformed into the Abstract Representation (AR), an intermediate representation that represents the control-flow graph of the program. The IKOS distribution provides a compiler frontend that transforms C/C++ programs into the AR using the LLVM compiler framework.

The next section illustrates how to install the required dependencies. 

### DEPENDENCIES

To build and run the analyzer, you will need the following dependencies:

* CMake >= 2.8.12.2
* GMP >= 4.3.1
* Boost >= 1.55
* Python 2 >= 2.7.3 or Python 3 >= 3.3
* SQLite >= 3.6.20
* LLVM and Clang >= 3.7
* A C++ compiler that supports C++14 (gcc >= 4.9.2 or clang >= 3.4)

Most of them can be installed using your package manager.

Installation instructions for Archlinux, CentOS, Debian, Fedora, Mac OS X, Red Hat and Ubuntu are available in the `docs` directory. These instructions assume you have sudo or root access. If you don't, please follow the instructions in `docs/INSTALL_ROOTLESS.md`.

Once you have all the required dependencies, move to the next section.

### BUILD AND INSTALL IKOS

Now that you have all the dependencies on your system, you can build and install IKOS.

As you open the IKOS distribution, you shall see the following content:

```
.
├── CMakeLists.txt
├── README.md
├── RELEASE_NOTES.md
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
$ cd build/
$ cmake -DCMAKE_INSTALL_PREFIX=/path/to/ikos-install-directory ..
$ make
$ make install
```

Note: We recommend to use Ninja to speed up the build process: https://ninja-build.org/
To build IKOS using Ninja, just add the option `-G Ninja` when you run cmake. Then use `ninja` instead of `make` as you build and install IKOS.

### RUNNING CTEST

To run the tests, we will need the full installation of IKOS. In addition, we will also need to build and install the verifier passes:

```
$ make verifier-passes
$ make install
```

Now, you can run the tests:

```
$ export IKOS_INSTALL=/path/to/ikos-install-directory
$ make test
```

HOW TO RUN IKOS
---------------

Suppose we want to analyze the following C program in a file, called *loop.c*:

```
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
$ export PATH=/path/to/ikos-install-directory/bin:$PATH
$ ikos -a boa --show-checks loop.c
```

Then you shall see the following output and that IKOS reports two occurrences of buffer overflow at line 8 and 9.

```
dlopen successful on /path/to/ikos-install-directory/lib/libpointer-shift-opt.dylib
Loaded ARBOS pass: ps-opt - Optimize pointer shift statements
dlopen successful on /path/to/ikos-install-directory/lib/libbranching-opt.dylib
Loaded ARBOS pass: branching-opt - Optimize the Control Flow Graph
dlopen successful on /path/to/ikos-install-directory/lib/libinline-init-gv.dylib
Loaded ARBOS pass: inline-init-gv - Inline initialization of global variables in main
dlopen successful on /path/to/ikos-install-directory/lib/libanalyzer.dylib
Loaded ARBOS pass: analyzer - Analyzer pass.
4 pass(es) registered.
Executing pass - ps-opt Optimize pointer shift statements
Executing pass - branching-opt Optimize the Control Flow Graph
Executing pass - inline-init-gv Inline initialization of global variables in main
Executing pass - analyzer Analyzer pass.
Running liveness variable analysis ...
Running function pointer analysis ...
** Generating pointer constraints ...
** Solving pointer constraints ...
Running pointer analysis ...
** Computing intra-procedural numerical invariants ...
** Generating pointer constraints ...
** Solving pointer constraints ...
Running value analysis ...
*** Analyzing function: main

Analysis timing report:
Liveness analysis: 0.02
Function pointer analysis: 0.01
Pointer analysis: 0.05
Value analysis: 0.02

Summary (per source code location):
Total number of checks                : 6
Total number of unreachable checks    : 0
Total number of safe checks           : 4
Total number of definite unsafe checks: 2
Total number of warnings              : 0

The program is definitely UNSAFE

Buffer overflow analysis checks:
| check     | context | file   | line | col | result |
+-----------+---------+--------+------+-----+--------+
| overflow  | .       | loop.c | 6    | 14  | ok     |
| underflow | .       | loop.c | 6    | 14  | ok     |
| overflow  | .       | loop.c | 8    | 10  | error  |
| underflow | .       | loop.c | 8    | 10  | ok     |
| overflow  | .       | loop.c | 9    | 18  | error  |
| underflow | .       | loop.c | 9    | 18  | ok     |
----------------------------------------------------------------------
BRUNCH_STAT Progress Get Analysis Results
BRUNCH_STAT arbos_plugins 0.20
BRUNCH_STAT ikos-pp 0.00
BRUNCH_STAT llvm-clang 0.03
BRUNCH_STAT llvm-to-ar 0.02
----------------------------------------------------------------------
```

The `ikos` command can also take LLVM bitcode as the input. You can use the following command to generate the LLVM bitcode for *loop.c*:

```
$ clang -emit-llvm -g -c loop.c -o loop.bc
```

The following shows the usage of the ikos command:

```
usage: ikos [options] file[.c|.bc]

positional arguments:
  file[.c|.bc]          The file to analyze

optional arguments:
  -h, --help            show this help message and exit
  -a {boa,uva,dbz,prover,nullity}, --analysis {boa,uva,dbz,prover,nullity}
                        Type of analysis (boa, dbz, uva, prover, nullity)
  -e ENTRY_POINTS, --entry-points ENTRY_POINTS
                        The entry point(s) of the program (default: main)
  --show-checks         Print analysis checks to standard output
  --ikos-pp             Enable all preprocessing optimizations
  --inline-all          Front-end inline all functions
  --intra               Run an intraprocedural analysis instead of an
                        interprocedural analysis
  --no-liveness         Do not run the liveness analysis
  --no-pointer          Do not run the pointer analysis
  -p {reg,ptr,mem}, --precision-level {reg,ptr,mem}
                        The precision level (reg, ptr, mem). Default to mem
  --gv-init-all         Initialize all global variables (default: initialize
                        scalars and pointers)
  --gv-init-scalars-only
                        Initialize only scalar global variables
  --gv-init-pointers-only
                        Initialize only pointer global variables
  --summaries           Use function summarization
  --pointer-summaries   Use function summarization for the pointer analysis
  --verify-bitcode      Verify LLVM bitcode is well formed
  --save-temps          Do not delete temporary files
  --temp-dir DIR        Temporary directory
  --display-invariants {all,fail,off}
                        Display invariants. Default to off
  --display-checks {all,fail,off}
                        Display checks. Default to off
  --output-db FILE, --db FILE
                        The output database file
  -d, --dot-cfg         Print CFG of all functions to 'dot' file
  -i, --ikosview        Show analysis results using ikosview GUI
  --cpu CPU             CPU time limit (seconds)
  --mem MEM             MEM limit (MB)
  --version             show ikos version
```

TROUBLESHOOTING
---------------

### IKOS could not find LLVM

If you used Homebrew on MAC OS X to install LLVM, remember to add the LLVM directory to your path:

```
$ export PATH="$(brew --prefix)/opt/llvm37/lib/llvm-3.7/bin:$PATH"
```

If you have a custom LLVM installation, you shall use the environment variable `LLVM_INSTALL`:

```
$ export LLVM_INSTALL=/path/to/llvm
```

OVERVIEW OF THE SOURCE CODE
---------------------------

The following illustrates the content of the root directory:

```
.
├── CMakeLists.txt
├── README.md
├── RELEASE_NOTES.md
├── abs-repr
├── analyzer
├── cmake
├── core
├── docs
├── frontends
└── scripts
```


`CMakeLists.txt` is the root CMake file.

`RELEASE_NOTES.md` contains the release notes for the latest versions.

`cmake` contains CMake files to search for related software libraries.

`core` contains the implementation of the theory of Abstract Interpretation, which includes the abstract domains, the fixpoint iterator, and various algorithms that support the implementation. More information can be found at `core/README.md`.

`abs-repr` contains the implementation of the ARBOS, a plugin framework that parses the AR and executes AR passes that perform various analysis. More information can be found at `abs-repr/README.md`.

`analyzer` contains the implementation of various analyses for specific defect detections. These analyses are implemented as AR passes and use the fixpoint iterator and abstract domains to perform analysis. More information can be found at `analyzer/README.md`.

`frontends` contains implementation of various frontend compilers to translate programs into AR. Currenlty we only support the LLVM frontend. More information can be found at `frontends/llvm/README.md`.

`scripts` contains the `bootstrap` script for a rootless installation.
