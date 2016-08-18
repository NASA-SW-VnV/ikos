LLVM Frontend
=============

This folder contains implementation of optimizations and translation from LLVM bitcode to an Abstract Representation (AR) program model using the LLVM framework as a compiler frontend. Prior to using this frontend, users must compile their programs into LLVM bitcode.

CONTENTS OF THIS FILE
---------------------

* Introduction
* Build and Install
* Running the LLVM Frontend Tool
* Overview of Source Code

INTRODUCTION
------------

The LLVM frontend takes an LLVM bitcode of a program, optimizes it to facilitate static analysis, and then translates it into the AR program model. The AR program model is represented in S-expressions in an _AR file_. You can find the AR specification in `../abs-repr/docs/AR_SPEC.md`.

BUILD AND INSTALL
-----------------

The LLVM frontend can be independently built. The source project uses cmake to generate build files needed by your build tool.

To build the LLVM frontend, run the following commands under `frontends/llvm`:

```
$ mkdir build
$ cd build
$ cmake -DLLVM_CONFIG_EXECUTABLE=/path/to/llvm/bin/llvm-config -DARBOS_ROOT=/path/to/arbos-install -DCMAKE_INSTALL_PREFIX=/path/to/install ..
$ make
$ make install
```

Note: We recommend to use Ninja to speed up the build process: https://ninja-build.org/
To build arbos using Ninja, just add the option `-G Ninja` when you run cmake, for instance:

```
$ cmake -G Ninja -DCMAKE_INSTALL_PREFIX=/path/to/install ..
```

Then, just use `ninja` everywhere instead of `make`.

Under `/path/to/install`, you should have the following directory structure:

```
├── bin
│   └── ikos-pp
└── lib
    └── llvm-to-ar.dylib (or llvm-to-ar.so on Linux)
```

`bin/ikos-pp` is an executable embedded with LLVM. It takes an LLVM bitcode and transforms it into another LLVM bitcode that is optimized to be more efficient for static analysis.

`lib/llvm-to-ar.dylib` is an LLVM pass to be used in the LLVM **opt** command. This LLVM pass translates the LLVM bitcode provided in the command line and prints out the AR program model in S-expresions in the console.

Running the LLVM Frontend Tools
-------------------------------

### bin/ikos-pp

Users can type the following to find the usage of this command:

```
$ ./bin/ikos-pp -help
```

The following shows the output with the '-help' option:

```
OVERVIEW: IkosPP -- LLVM bitcode Pre-Processor for Static Analysis

USAGE: ikos-pp [options] <input LLVM bitcode file>

OPTIONS:

General options:

  -S                                      - Write output as LLVM assembly
  -bounds-checking-single-trap            - Use one trap block per function
  -default-data-layout=<layout-string>    - data layout string to use if not specified by module
  -enable-load-pre                        -
  -enable-objc-arc-opts                   - enable/disable all ARC Optimizations
  -enable-scoped-noalias                  -
  -enable-tbaa                            -
  -ikospp-inline-all                      - Inline all functions
  -ikospp-level                           - Choose preprocessing level:
    =simple                               -   Only passes required for correctness
    =full                                 -   Enable all optimizations
  -ikospp-verify                          - Verify the module is well formed upon all transformations
  -internalize-public-api-file=<filename> - A file containing list of symbol names to preserve
  -internalize-public-api-list=<list>     - A list of symbol names to preserve
  -join-liveintervals                     - Coalesce copies (default=true)
  -o=<filename>                           - Override output filename
  -oll=<filename>                         - Output analyzed bitcode
  -print-after-all                        - Print IR after each pass
  -print-before-all                       - Print IR before each pass
  -rewrite-map-file=<filename>            - Symbol Rewrite Map
  -rng-seed=<seed>                        - Seed for the random number generator
  -sroa-threshold=<int>                   - Threshold for ScalarReplAggregates pass
  -stackmap-version=<int>                 - Specify the stackmap encoding version (default = 1)
  -stats                                  - Enable statistics output from program (available with Asserts)
  -time-passes                            - Time each pass, printing elapsed time for each on exit
  -verify-debug-info                      -
  -verify-dom-info                        - Verify dominator info (time consuming)
  -verify-loop-info                       - Verify loop info (time consuming)
  -verify-region-info                     - Verify region info (time consuming)
  -verify-scev                            - Verify ScalarEvolution's backedge taken counts (slow)

Generic Options:

  -help                                   - Display available options (-help-hidden for more)
  -help-list                              - Display list of available options (-help-list-hidden for more)
  -version                                - Display the version of this program
```

### lib/llvm-to-ar.dylib

The following illustrates the command to translate _file.bc_ (LLVM bitcode) to _file.ar_ (AR in S-expresions):

```
$ opt -load=/path/to/lib/llvm-to-ar.dylib -arbos < file.bc > file.ar
```

Overview of the Source Code
---------------------------

The following illustrates the directory structure in this folder:

```
.
├── CMakeLists.txt
├── README.md
├── include
│   ├── passes
│   └── utils
└── src
    ├── ikos-pp
    ├── passes
    └── utils
```

`include/passes` and `src/passes` contain implementation of our LLVM passes for optimization and translation.

`include/utils` and `src/utils` contain implementation of utility functions that support optimization.

`src/ikos-pp` contains implemention of `ikos-pp`.
