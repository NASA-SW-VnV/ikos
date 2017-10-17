LLVM Frontend
=============

This folder contains implementation of optimizations and translation from LLVM bitcode to an Abstract Representation (AR) program model using the LLVM framework as a compiler frontend. Prior to using this frontend, users must compile their programs into LLVM bitcode.

Introduction
------------

The LLVM frontend takes an LLVM bitcode of a program, optimizes it to facilitate static analysis, and then translates it into the AR program model. The AR program model is represented in S-expressions in an _AR file_. You can find the AR specification in [../abs-repr/docs/AR_SPEC.md](../abs-repr/docs/AR_SPEC.md).

Build and Install
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

  -S                                              - Write output as LLVM assembly
  -bounds-checking-single-trap                    - Use one trap block per function
  -cvp-dont-process-adds                          -
  -default-data-layout=<layout-string>            - Data layout string to use if not specified by module
  -enable-load-pre                                -
  -enable-name-compression                        - Enable name string compression
  -enable-objc-arc-opts                           - enable/disable all ARC Optimizations
  -enable-scoped-noalias                          -
  -enable-tbaa                                    -
  -entry-points=<string>                          - List of entry points
  -expensive-combines                             - Enable expensive instruction combines
  -filter-print-funcs=<function names>            - Only print IR for functions whose name match this for all print-[before|after][-all] options
  -hash-based-counter-split                       - Rename counter variable of a comdat function based on cfg hash
  -ignore-empty-index-file                        - Ignore an empty index file and perform non-ThinLTO compilation
  -ikospp-inline-all                              - Inline all functions
  -ikospp-level                                   - Choose preprocessing level:
    =simple                                       -   Only passes required for correctness
    =full                                         -   Enable all optimizations
  -ikospp-verify                                  - Verify the module is well formed upon all transformations
  -internalize-public-api-file=<filename>         - A file containing list of symbol names to preserve
  -internalize-public-api-list=<list>             - A list of symbol names to preserve
  -join-liveintervals                             - Coalesce copies (default=true)
  -no-discriminators                              - Disable generation of discriminator information.
  -o=<filename>                                   - Override output filename
  -oll=<filename>                                 - Output analyzed bitcode
  -print-after-all                                - Print IR after each pass
  -print-before-all                               - Print IR before each pass
  -rewrite-map-file=<filename>                    - Symbol Rewrite Map
  -rng-seed=<seed>                                - Seed for the random number generator
  -sample-profile-check-record-coverage=<N>       - Emit a warning if less than N% of records in the input profile are matched to the IR.
  -sample-profile-check-sample-coverage=<N>       - Emit a warning if less than N% of samples in the input profile are matched to the IR.
  -sample-profile-inline-hot-threshold=<N>        - Inlined functions that account for more than N% of all samples collected in the parent function, will be inlined again.
  -sample-profile-max-propagate-iterations=<uint> - Maximum number of iterations to go through when propagating sample block/edge weights through the CFG.
  -stackmap-version=<int>                         - Specify the stackmap encoding version (default = 2)
  -static-func-full-module-prefix                 - Use full module build paths in the profile counter names for static functions.
  -stats                                          - Enable statistics output from program (available with Asserts)
  -stats-json                                     - Display statistics as json data
  -summary-file=<string>                          - The summary file to use for function importing.
  -time-passes                                    - Time each pass, printing elapsed time for each on exit
  -verify-debug-info                              -
  -verify-dom-info                                - Verify dominator info (time consuming)
  -verify-loop-info                               - Verify loop info (time consuming)
  -verify-loop-lcssa                              - Verify loop lcssa form (time consuming)
  -verify-machine-dom-info                        - Verify machine dominator info (time consuming)
  -verify-region-info                             - Verify region info (time consuming)
  -verify-scev                                    - Verify ScalarEvolution's backedge taken counts (slow)
  -verify-scev-maps                               - Verify no dangling value in ScalarEvolution's ExprValueMap (slow)
  -vp-counters-per-site=<number>                  - The average number of profile counters allocated per value profiling site.
  -vp-static-alloc                                - Do static counter allocation for value profiler

Generic Options:

  -help                                           - Display available options (-help-hidden for more)
  -help-list                                      - Display list of available options (-help-list-hidden for more)
  -version                                        - Display the version of this program
```

### lib/llvm-to-ar.dylib

The following illustrates the command to translate _file.bc_ (LLVM bitcode) to _file.ar_ (AR in S-expresions):

```
$ opt -load=/path/to/lib/llvm-to-ar.dylib -arbos < file.bc > file.ar
```

Overview of the source code
---------------------------

The following illustrates the directory structure in this folder:

```
.
├── CMakeLists.txt
├── README.md
├── include
│   └── passes
└── src
    ├── ikos-pp
    └── passes
```

[include/passes](include/passes) and [src/passes](src/passes) contain the implementation of our LLVM passes for optimization and translation.

[src/ikos-pp](src/ikos-pp) contains the implementation of `ikos-pp`.
