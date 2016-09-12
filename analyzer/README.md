ANALYZER
========

This folder contains the implementation of the analyzer.

CONTENTS OF THIS FILE
---------------------

* Introduction
* Build and Install
* How to run IKOS
* Documentation
* Overview of Source Code

INTRODUCTION
------------

The IKOS analyzer (previously called IkosCC) is an abstract interpretation-based static analyzer that aims at proving absence of certain kind of runtime errors in C and C++ programs:

* **boa**: buffer overflow (out-of-bound array indexing)
* **dbz**: integer division by zero
* **nullity**: null pointer dereference
* **uva**: read of uninitialized integer variables
* **prover**: violation of user-defined properties to prove additional runtime properties (similar to C `assert`)

BUILD AND INSTALL
-----------------

We really recommend to build the analyzer from the root directory of the distribution, but it is still possible to build all components (arbos, the llvm frontend and the analyzer) independently. To do so, first follow the instructions in `abs-repr/README.md`, `frontends/llvm/README.md` and `core/README.md`.

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

Please check the root `README.md` to find more instructions.

### BUILD AND INSTALL THE ANALYZER

Once you have installed all other components, run the following commands in the `analyzer` directory:

```
$ mkdir build
$ cd build
$ cmake \
    -DCMAKE_INSTALL_PREFIX=/path/to/analyzer-installation-directory \
    -DIKOS_ROOT=/path/to/ikos-installation-directory \
    -DARBOS_ROOT=/path/to/arbos-installation-directory \
    ..
$ make
$ make install
```

### RUNNING CTEST

To run the tests, first build and install ikos. Then, run the following commands under the `build` directory:

```
$ export IKOS_INSTALL=/path/to/ikos-installation
$ make test
```

HOW TO RUN IKOS
===============

First, take a look at the `README.md` in the root directory of the distribution.

Using the following commands, you can run IKOS to detect buffer overflow defects directly against a C program called *loop.c*:

```
$ export PATH=/path/to/ikos-installation/bin:$PATH
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

The column `check` describes the type of kind of check (buffer access in this case): `overflow` for accessing an element past the end of a memory block and `underflow` for an access with a negative offset.
The column `context` shows the call stack of at the time the analysis performed the check. The symbol `.` denotes the `main` function. Each caller (element of the call stack) is denoted by `fun_name@callsite_line@callsite_col` separated by the symbol `/` where `fun_name` is the function name of the caller, `callsite_line` is the line number of the call site and `callsite_col` is the column number of the call site.
The `file`, `line` and `col` columns give the location of the operation checked in the original source code.
The column `result` describes the conclusion of the static analyzer on the check:

* **ok** means that the buffer access is safe for all execution contexts;
* **error** means that the buffer access always results into an error, regardless of the execution context;
* **warning** may mean two things:
   1. the operation results into an error for some execution contexts but not other, or
   2. the static analyzer did not have enough information to conclude, because either the program does not provide enough information (check dependent on the value of an external input for example) or the static analysis algorithms are not powerful enough;
* **unreachable** means that the code in which the buffer operation is located is never executed (dead code).

Since the number of checks is usually large, ikos does not show this information by default. To enforce that, it is necessary to include the option `--show-checks`. If the user does not enable this option, ikos will still output a summary about the analysis result:

    Interprocedural buffer overflow summary:
    Total number of checks                : 256
    Total number of unreachable checks    : 0
    Total number of safe checks           : 256
    Total number of definite unsafe checks: 0
    Total number of warnings              : 0

    The program is SAFE

For large programs, we recommend the use of the option `--ikos-pp`:

```
$ ikos -a boa --ikos-pp --show-checks prog.bc
```

The option `ikos-pp` runs Ikos-PP, a preprocessor of LLVM bitecode that facilitates the task of static analysis. IkosPP performs a set of LLVM bitcode transformations that can improve both the precision of the subsequent analyses as well as performance.

### Compiling a whole C project with LLVM to generate a single .bc file

This requires modifying the Makefile used to build the program so that the compiler tools invoked are those provided by LLVM. This usually amounts to changing the settings for the Makefile variables `CC`, `LD` and `AR`.

For instance, given the following Makefile sample:

    CC = gcc -c
    LD = gcc
    AR = ar

    LIB_FILES = f1.o f2.o

    all: main.o lib.a
         $(LD) -o example main.o lib.a

    %.o: %.c
         $(CC) -o $@ $<

    lib.a: $(LIB_FILES)
           $(AR) rs $@ $(LIB_FILES)

Modifying this Makefile so that it can be compiled by the LLVM front-end only requires changing the settings of the compiler variables as follows:

    CC = path/to/clang -c -g -emit-llvm -fno-inline
    LD = /path/to/llvm-ld -link-as-library -disable-inlining -disable-opt
    AR = /path/to/llvm-ar

and the rest of the Makefile is unchanged.  The binary `example` now contains LLVM bitecode rather than machine executable code so can be processed by IkosCC using the python script `ikos`.

The settings listed above are generic and can be used in any Makefile that uses `CC`, `LD` and `AR`. If the Makefile directly invokes the compiler tools, each invocation of `gcc`, ld or ar shall be manually modified.

### Analyses Options

This section describes the most relevant options when using the ikos analyzer.

#### Inter-procedural vs Intra-procedural

An **inter-procedural** analysis analyzes a function considering its call stack while an **intra-procedural** analysis ignores it. The former produces more precise results than the latter but it is often much more expensive. Another way of saying the same is that an inter-procedural analysis will consider only **valid** paths. A path is valid if it respects the fact that when a procedure finishes it returns to the site of the most recent call.

IKOS implements inter-procedural analysis by inlining function calls. All functions are inlined except:

* recursive functions
* variable argument list calls

IKOS uses an inter-procedural analysis by default. Provide `--intra` if you want to run an intra-procedural analysis.

#### Numerical abstract domains

IKOS relies on a **value analysis** that is parametric on the numerical domain used ultimately to model each program variable. Currently, the ikos analyzer only models integer variables. Floating point variables are safely ignored.

The current numerical domains are:

* **intervals**: expresses relationships of the form x <= k where x is a variable a k is a (possible negative) constant. For instance, we can express that the variable x is between -10 and 23 with the two constraints x <= 23 and -x <= -10.

* **intervals** with **congruences**: expresses interval relationships as well as congruences relationships. A congruence relationship maps a variable to a set of values aZ + b (where a is positive integer and b is an integer). The expression aZ + b means all integers that are congruent to b modulo a. That is, { x | x \in Z, x = b (mod a) }.

    Recall that n = m (mod c) means that n is congruent to m modulo c. Informally, n
    and m have the same remainder when divided by c. For instance,

    * the set of even numbers is {...,-4,-2,0,2,4,...} and is represented
       by 2Z + 0 (i.e., {x | x \in Z, x = 0 (mod 2)})

    * the set of odd numbers is {...,-3,-1,1,3,5,...} is represented
       by 2Z + 1 (i.e., {x | x \in Z, x = 1 (mod 2)})

* **dbm**: expresses relationships between two variables x - y <= k where x and y are positive variables and k is a (possible negative) constant. This domain can also represent equalities (e.g., x = y iff x - y <= 0 and y - x <= 0)

* **dbm** with **variable packing**: expresses relationships between two variables x - y <= k. That domain uses a variable packing algorithm to optimize the performance and should offer almost the same precision as the simple `dbm` abstract domain.

* **octagons**: expresses relationships between two variables x - y <= k where x and y are (possible negative) variables and k is a (possible negative) constant.

Abstract domains such as `intervals` and `congruences` are called **non-relational** while `dbm` and `octagons` are called (weakly) **relational** domains.

The abstract domain is a compile-time option for IKOS. You shall provide the option `-DABSTRACT_DOMAIN=` while running cmake. Available choices are: `INTERVAL`, `CONGRUENCE`, `INTERVAL_CONGRUENCE`, `OCTAGON`, `DBM`, `VAR_PACKING_DBM`. For instance:

```
$ cmake -DCMAKE_INSTALL_PREFIX=/path/to/ikos-installation-directory -DABSTRACT_DOMAIN=DBM ...
```

And then run make and make install, as usual.

#### Degree of precision

Each analysis can be executed using one of the following levels of precision, presented from the coarsest (and cheaper) to the most precise (and most expensive):

* **reg**: models only integer scalars.
* **ptr**: level `reg` and it models pointer addresses (but not
contents).
* **mem**: level `ptr` and it models pointer contents.

By default, ikos uses the precision `mem`. Provide `-p {reg,ptr,mem}` if you want to use another level of precision.

DOCUMENTATION
-------------

To generate API documentation using Doxygen, run the following command in the `build` directory:

```
make docs
```

OVERVIEW OF THE SOURCE CODE
---------------------------

This folder contains the core of all the analyses which are implemented as an ARBOS plugin in `src/ar-passes/analyzer.cpp`.

Important considerations:

* All the analyses take ARBOS CFGs (as defined in `include/analyzer/ar-wrapper/cfg.hpp`).
* All the analyses should manipulate ARBOS AR only via namespace `ar` in `include/analyzer/ar-wrapper/wrapper.hpp`.

#### docs/

`docs/doxygen` contains Doxygen files.

#### include/

`include/analyzer/config.hpp`: types declaration for numerical abstract domains

##### include/analyzer/analysis

`include/analyzer/analysis/common.hpp`: common types and declarations for all the analyses. Among them, it is specially important a variable name factory.

`include/analyzer/analysis/context.hpp`: class to propagate the global state of the analyses.

`include/analyzer/analysis/liveness.hpp`: computes the set of live variables for an ARBOS CFG. This can be used optionally by the analysis to remove dead variables from the fixpoint computation.

`include/analyzer/analysis/pointer.hpp`: computes for each pointer variable in the ARBOS CFG the set of memory locations to which it may point-to. This is used as a pre-step for improving the precision of the other analyses.

`include/analyzer/analysis/sym_exec_api.hpp`:
   * API `sym_exec` to perform the abstract transfer function to each ARBOS AR instruction.
   * API `sym_exec_call` to analyze call sites. Each implementation of this API should target different inter-procedural strategies (e.g., context-insensitive, inlining, summary-based, etc).
   * default implementation `context_insensitive_sym_exec_call` for context-insensitive analysis.

`include/analyzer/analysis/num_sym_exec.hpp`: this file defines class `num_sym_exec` (inherits from `sym_exec`) and it is a very important class since most analyses rely on it to perform the abstract transfer function.  This class executes each ARBOS instruction using the API of a value analysis. The class is parametric in the value analysis. An implementation of a value analysis is in the directory `include/analyzer/domains/value_domain.hpp`. The value analysis can be a simple numerical abstract domain so only integer scalars can be modelled or it can be a more sophisticated domain keeping track of pointer offsets and memory contents. The level of precision is chosen by the user.

`include/analyzer/analysis/inliner.hpp`: derives from `sym_exec_call` and implements the inlining strategy.

##### include/analyzer/ar-wrapper

This folder contains API's to access and transform `arbos` AR.

`include/analyzer/ar-wrapper/cfg.hpp`: build a CFG from an ARBOS function. The CFG provides an API which is compatible with the IKOS fixpoint algorithms but each basic block still contains AR statements. The ARBOS CFG is also augmented with useful information for dataflow analyses such as the set of used and defined variables.

`include/analyzer/ar-wrapper/literal.hpp`: class that converts an Arbos operand to an ARBOS-independent format.

`include/analyzer/ar-wrapper/wrapper.hpp`: implements an adaptor (namespace `ar`) for accessing to ARBOS AR. The purpose is to establish a layer of separation between ARBOS AR and all the analyses (`include/analyzer/analysis`).

`include/analyzer/ar-wrapper/transformations.hpp`: implements an adaptor (namespace `transformations`) for modifying ARBOS AR with similar motivation than
above.

##### include/analyzer/checkers

This folder contains classes that check for properties on the code (out-of-bound accesses, divisions by zero, etc), given the result of an analysis.

##### include/analyzer/ikos-wrapper

`include/analyzer/ikos-wrapper/iterators`: wrapper to the IKOS fixpoints. It provides:
   * forward abstract intepreter
   * backward abstract interpreter
  The backward abstract interpreter is limited to dataflow analyses, otherwise although sound it will be too imprecise.

##### include/analyzer/domains

This folder contains analysis-specific abstract domains not available in ikos core library.

##### include/analyzer/utils

Common utilities for the analyses.

##### include/analyzer/examples

`include/analyzer/examples/muaz.hpp`: a micro language for semantic modelling of arrays and integer numbers.

#### scripts

`scripts/ikos`: ikos analyzer python script

#### src

#### src/ar-passes

`src/ar-passes/analyzer.cpp`: Analyzer ARBOS pass. This is the entry point for all analyses.

`src/ar-passes/ar_to_dot.cpp`: ARBOS pass that translates AR code into dot format

`src/ar-passes/inline_init_gv.cpp`: Inline initialization of global variables to the main function.

`src/ar-passes/pointer_shift_opt.cpp`: Remove redundant arithmetic statements produced by the translation of LLVM getElementPtr instructions to ARBOS pointer shifts.
