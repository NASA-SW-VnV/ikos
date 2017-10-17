ANALYZER
========

This folder contains the implementation of the analyzer.

Introduction
------------

The IKOS analyzer (previously called **IkosCC**) is an abstract interpretation-based static analyzer that aims at proving absence of certain kind of runtime errors in C and C++ programs:

* **boa**: buffer overflow (out-of-bound array indexing)
* **dbz**: integer division by zero
* **nullity**: null pointer dereference
* **uva**: read of uninitialized variables
* **upa**: unaligned pointer dereference
* **prover**: violation of user-defined properties to prove additional runtime properties (similar to C `assert`)

Build and Install
-----------------

We really recommend to build the analyzer from the root directory of the distribution, but it is still possible to build all components (arbos, the llvm frontend and the analyzer) independently. To do so, first follow the instructions in [abs-repr/README.md](../abs-repr/README.md), [frontends/llvm/README.md](../frontends/llvm/README.md) and [core/README.md](../core/README.md).

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

Please check the root [README.md](../README.md#dependencies) to find more instructions.

### Build and Install the analyzer

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

### Running the tests

To run the tests, first build and install ikos. Then, run the following commands under the `build` directory:

```
$ PATH=/path/to/ikos-installation/bin:$PATH
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
$ PATH=/path/to/ikos-installation/bin:$PATH
$ ikos loop.c --export
```

Then you shall see the following output and that IKOS reports two occurrences of buffer overflow at line 8 and 9.

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

To analyze a large program, you shall run the `ikos` command first to create the result database `output.db` and then generate a report using the `ikos-render` command. This way, you can generate reports in different formats and adjust the verbosity to your needs, without running the analysis again. See below to learn more about export options.

### Export Formats

This section describes the different report formats.

#### GCC Format

By default, the `--export` option generates a report in a gcc-style format, convenient for a terminal.

The report is composed of a list of statement reports. Here is an example of a statement report:

```
test-23-safe.c: In function 'foo':
test-23-safe.c:8:5: safe: safe statement
    a[i] = 'A';
    ^
test-23-safe.c:8:5: note: called from:
test-23-safe.c:19:13: function 'f'
  char* A = foo(str, 10);
            ^
test-23-safe.c:8:5: note: called from:
test-23-safe.c:23:13: function 'g'
  char* C = foo(B, 10);
            ^
```

This is the statement report for line 8, column 5 of [test-23-safe.c](tests/regression/boa/test-23-safe.c). Each statement report has one of the following status:

* **safe**: the statement is safe (free of undefined behavior);
* **error**: the statement always results into an error;
* **warning** may mean two things:
   1. the operation results into an error for some executions, or
   2. the static analyzer did not have enough information to conclude, because either the program does not provide enough information (check dependent on the value of an external input for example) or the static analysis algorithms are not powerful enough;
* **unreachable**: the statement is never executed (dead code);
* **note**: an additional notice from the analysis.

Each statement report is valid for some calling contexts. A calling context is a list of function calls that leads to a specific function. In the example above, the statement `a[i] = 'A';` is safe whether the function `foo` has been called from `f`, line 19 or from `g`, line 23. If the report doesn't show the calling contexts, that means the report is valid for all possible calling contexts.

#### JSON Format

You can export the report in JSON, using the option `--export-format=json`.

See [docs/FORMAT_JSON.md](docs/FORMAT_JSON.md) for more details.

### Running IKOS on a whole C/C++ project

To run ikos on a large project, you will first need to compile the program into LLVM bitcode (`.bc`). The LLVM bitcode is a generic assembly language that can be used as an intermediate representation for any compiled language.

#### Compiling your project into a single .bc file

The easiest way to compile your project into a `.bc` file is to use the tool **Whole Program LLVM**: https://github.com/travitch/whole-program-llvm

You can install it easily using pip:

```
$ pip install wllvm
```

Then, you will need to set the following environment variable:

```
$ export LLVM_COMPILER=clang
```

Now, just use `wllvm` and `wllvm++` as your C and C++ compilers. These are wrappers that invoke the compiler as normal, and then, for each object file, create the equivalent LLVM bitcode file. This way, you can actually build your project as you usually do, and extract the final LLVM bitcode afterwards. It is also independent from any build system (autoconfig, cmake, Makefiles, ..).

For instance, using autoconf, you shall run:

```
$ CC=wllvm CXX=wllvm++ ./configure
$ make
```

Once everything is built, you can extract the LLVM bitcode file from a binary using the extract-bc command:

```
$ extract-bc prog
```

It will produce the LLVM bitcode file `prog.bc`. You can now analyze it using ikos:

```
$ ikos --ikos-pp prog.bc
```

#### Using ikos-pp

Note that for large programs, we recommend to use the option `--ikos-pp`:

```
$ ikos --ikos-pp prog.bc
```

The option `--ikos-pp` runs IKOS-PP, a preprocessor of LLVM bitcode that facilitates the task of static analysis. IKOS-PP performs a set of LLVM bitcode transformations that can improve both the precision of the subsequent analyses as well as performance. Unfortunately, it might also hide errors in your code (undefined behavior) because of optimizations.

### Analyses Options

This section describes the most relevant options of the analyzer.

#### Type of analyses

By default, ikos runs a default set of analyses: boa, dbz, nullity and prover. If you want to run specific analyses, use the `-a` parameter:

```
$ ikos -a boa -a nullity --export test.c
```

The list of available analyses is:

* **Buffer Overflow Analysis (boa)**: checks for out-of-bound array accesses
* **Division By Zero (dbz)**: checks for integer divisions by zero
* **Null Pointer Analysis (nullity)**: checks for null pointer dereferences
* **Uninitialized Variable Analysis (uva)**: checks for read of uninitialized variables
* **Unaligned Pointer Analysis (upa)**: checks for unaligned pointer dereferences
* **Assertion Prover (prover)**: Prove user-defined properties, using `__ikos_assert(condition)`

Notes:

* **uva** is disabled by default because this analysis currently generates a lot of false positives. It will be revisited in the future.
* **upa** needs a congruence domain to generate meaningful results. See [Numerical abstract domains](#numerical-abstract-domains) and consider using the `INTERVAL_CONGRUENCE` domain.

#### Entry points

By default, ikos assumes the entry point of the program is `main`. You can specify a list of entry points using the `-e` parameter:

```
$ ikos -e f -e g test.c
```

#### ikos-pp

The option `--ikos-pp` runs IKOS-PP, a preprocessor of LLVM bitcode that facilitates the task of static analysis. IKOS-PP performs a set of LLVM bitcode transformations that can improve both the precision of the subsequent analyses as well as performance.

#### Inter-procedural vs Intra-procedural

An **inter-procedural** analysis analyzes a function considering its call stack while an **intra-procedural** analysis ignores it. The former produces more precise results than the latter but it is often much more expensive. Another way of saying the same is that an inter-procedural analysis will consider only **valid** paths. A path is valid if it respects the fact that when a procedure finishes it returns to the site of the most recent call.

IKOS implements inter-procedural analysis by inlining function calls. All functions are inlined except:

* recursive functions
* variable argument list calls

IKOS uses an inter-procedural analysis by default. Provide `--intra` if you want to run an intra-procedural analysis.

#### Static inlining, Dynamic Inlining, Summaries

By default, ikos uses **dynamic inlining** rather than static inlining or summaries.

* **dynamic inlining**: the analysis exploring a call site by recursively analyzing the callee function
* **static inlining**: the frontend is responsible for inlining all function calls (this is not always possible)
* **summaries**: each function is analyzed independently with a relational domain, to build a summary. Then, a top-down analysis is performed, using these summaries and propagating the calling contexts.

Tu use static inlining, use the parameter `--inline-all`. To use function summarization, use `--summaries`.

#### Numerical abstract domains

IKOS relies on a **value analysis** that is parametric on the numerical domain used ultimately to model each program variable. Currently, the ikos analyzer only models integer and pointer variables. Floating point variables are safely ignored.

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

The abstract domain is a compile-time option for IKOS. You shall provide the option `-DABSTRACT_DOMAIN=` while running cmake. For instance:

```
$ cmake -DCMAKE_INSTALL_PREFIX=/path/to/ikos-installation-directory -DABSTRACT_DOMAIN=DBM ...
```

And then run make and make install, as usual.

Available choices are:
* `INTERVAL`: interval domain, [CC77](https://www.di.ens.fr/~cousot/COUSOTpapers/publications.www/CousotCousot-POPL-77-ACM-p238--252-1977.pdf)
* `CONGRUENCE`: congruence domain, [Gra89](http://www.tandfonline.com/doi/abs/10.1080/00207168908803778)
* `INTERVAL_CONGRUENCE`: reduced product of `INTERVAL` and `CONGRUENCE`
* `OCTAGON`: octagon domain, [AST01](https://www-apr.lip6.fr/~mine/publi/article-mine-ast01.pdf)
* `DBM`: domain based on difference-bound matrices, [PADO01](https://www-apr.lip6.fr/~mine/publi/article-mine-padoII.pdf)
* `VAR_PACKING_DBM`: difference-bound matrices with variable packing, [VMCAI16](https://seahorn.github.io/papers/vmcai16.pdf)
* `VAR_PACKING_DBM_CONGRUENCE`: reduced product of `VAR_PACKING_DBM` and `CONGRUENCE`
* `GAUGE`: gauge domain, [CAV12](https://ti.arc.nasa.gov/publications/4767/download/)
* `GAUGE_INTERVAL_CONGRUENCE`: reduced product of `GAUGE`, `INTERVAL` and `CONGRUENCE`

##### Using APRON

[APRON](http://apron.cri.ensmp.fr/library/) is a C library for static analysis using Abstract Interpretation. It implements several complex abstract domains, such as the Polyhedra domain.

IKOS provides a wrapper for APRON, allowing you to use any APRON domain for the analysis.

To use APRON, first download, build and install it. Consider using the svn trunk. You will also need to build APRON with [Parma Polyhedra Library](http://bugseng.com/products/ppl/) enabled. Set `HAS_PPL = 1` and define `PPL_PREFIX` in your `Makefile.config`

Now, to use APRON, just provide the option `-DAPRON_ROOT=/path/to/apron-install`, and specify the abstract domain you want to use with `-DABSTRACT_DOMAIN`. For instance:

```
cmake -DCMAKE_INSTALL_PREFIX=/path/to/ikos-install \
    -DAPRON_ROOT=/path/to/apron-install \
    -DABSTRACT_DOMAIN=APRON_PPL_POLYHEDRA \
    ..
```

Available choices are:
* [APRON_INTERVAL](http://apron.cri.ensmp.fr/library/0.9.10/apron/apron_21.html#SEC54)
* [APRON_OCTAGON](http://apron.cri.ensmp.fr/library/0.9.10/apron/oct_doc.html)
* [APRON_POLKA_POLYHEDRA](http://apron.cri.ensmp.fr/library/0.9.10/apron/apron_25.html#SEC58)
* [APRON_POLKA_LINEAR_EQUALITIES](http://apron.cri.ensmp.fr/library/0.9.10/apron/apron_25.html#SEC58)
* [APRON_PPL_POLYHEDRA](http://apron.cri.ensmp.fr/library/0.9.10/apron/apron_29.html#SEC65)
* [APRON_PPL_LINEAR_CONGRUENCES](http://apron.cri.ensmp.fr/library/0.9.10/apron/apron_29.html#SEC65)
* [APRON_PKGRID_POLYHEDRA_LIN_CONGRUENCES](http://apron.cri.ensmp.fr/library/0.9.10/apron/apron_33.html#SEC69)

#### Degree of precision

Each analysis can be executed using one of the following levels of precision, presented from the coarsest (and cheaper) to the most precise (and most expensive):

* **reg**: models only integer scalars.
* **ptr**: level `reg` and it models pointer addresses (but not
contents).
* **mem**: level `ptr` and it models pointer contents.

By default, ikos uses the precision `mem`. Provide `-p {reg,ptr,mem}` if you want to use another level of precision.

#### Other analysis options

* `--show-raw-checks`: print the content of the result database;
* `--no-liveness`: disable the liveness analysis pass;
* `--no-pointer`: disable the pointer analysis pass;
* `--dot-cfg`: print ARBOS CFG to a .dot file (very useful for debugging);
* `--ikosview`: show analysis results using the ikosview GUI (note that ikosview is currently not open source).

See `ikos --help` for more information.

### Export Options

This section describes the options of the export feature.

#### Export format

You can chose the output format using the `--export-format` option. Available values are:

* **csv**: CSV format, convenient for spreadsheet import;
* **gcc**: a gcc-style format, convenient for the terminal;
* **json**: JSON format, convenient for developers.

#### Export file

By default, the report is generated on the standard output. You can write it into a file using `--export-file=/path/to/report`

#### Export level

The `--export-level` option allows you to filter unwanted reports. It will ignore all reports that have a status less critical that the one provided.

Possible values are: all = safe < note < warning < error.

By default, the export level is `warning`, thus you only get warnings and errors. If you want all reports, use `--export-level=all`.

#### Export verbosity

Use `--export-verbosity [1-4]` to specify the verbosity you want. A verbosity of one will give you very short messages, where a verbosity of 4 will provide you all information the analyzer has.

Note: It has no effect for the JSON format.

#### Other export options

* `--export-no-unreachable`: ignore unreachable reports;
* `--export-demangle`: demangle C++ symbols (JSON format only).

See `ikos-render --help` for more information.

Documentation
-------------

To generate API documentation using Doxygen, run the following command in the `build` directory:

```
make docs
```

Overview of the source code
---------------------------

This folder contains the core of all the analyses which are implemented as an ARBOS plugin in [src/ar-passes/analyzer.cpp](src/ar-passes/analyzer.cpp).

Important considerations:

* All the analyses take ARBOS CFGs (as defined in [include/analyzer/ar-wrapper/cfg.hpp](include/analyzer/ar-wrapper/cfg.hpp)).
* All the analyses should manipulate ARBOS AR only via namespace `ar` in [include/analyzer/ar-wrapper/wrapper.hpp](include/analyzer/ar-wrapper/wrapper.hpp).

#### docs/

* [docs/FORMAT_JSON.md](docs/FORMAT_JSON.md) documents the JSON format
* [docs/doxygen](docs/doxygen) contains Doxygen files.

#### include/

* [include/analyzer/config.hpp](include/analyzer/config.hpp): types declaration for numerical abstract domains

##### include/analyzer/analysis

* [include/analyzer/analysis/common.hpp](include/analyzer/analysis/common.hpp): common types and declarations for all the analyses.

* [include/analyzer/analysis/context.hpp](include/analyzer/analysis/context.hpp): class to propagate the global state of the analyses.

* [include/analyzer/analysis/variable_name.hpp](include/analyzer/analysis/variable_name.hpp): defines the different variable names (local, global, etc), and the variable name factory.

* [include/analyzer/analysis/liveness.hpp](include/analyzer/analysis/liveness.hpp): computes the set of live variables for an ARBOS CFG. This can be used optionally by the analysis to remove dead variables from the fixpoint computation.

* [include/analyzer/analysis/pointer.hpp](include/analyzer/analysis/pointer.hpp): computes for each pointer variable in the ARBOS CFG the set of memory locations to which it may point-to. This is used as a pre-step for improving the precision of the other analyses.

* [include/analyzer/analysis/sym_exec_api.hpp](include/analyzer/analysis/sym_exec_api.hpp):
   * API `sym_exec` to perform the abstract transfer function to each ARBOS AR instruction.
   * API `sym_exec_call` to analyze call sites. Each implementation of this API should target different inter-procedural strategies (e.g., context-insensitive, inlining, summary-based, etc).
   * default implementation `context_insensitive_sym_exec_call` for context-insensitive analysis.

* [include/analyzer/analysis/num_sym_exec.hpp](include/analyzer/analysis/num_sym_exec.hpp): this file defines class `num_sym_exec` (inherits from `sym_exec`) and it is a very important class since most analyses rely on it to perform the abstract transfer function.  This class executes each ARBOS instruction using the API of a value analysis. The class is parametric in the value analysis. An implementation of a value analysis is in [core/include/ikos/domains/value_domain.hpp](../core/include/ikos/domains/value_domain.hpp). The value analysis can be a simple numerical abstract domain so only integer scalars can be modelled or it can be a more sophisticated domain keeping track of pointer offsets and memory contents. The level of precision is chosen by the user.

* [include/analyzer/analysis/inliner.hpp](include/analyzer/analysis/inliner.hpp): derives from `sym_exec_call` and implements the inlining strategy.

##### include/analyzer/ar-wrapper

This folder contains API's to access and transform `arbos` AR.

* [include/analyzer/ar-wrapper/cfg.hpp](include/analyzer/ar-wrapper/cfg.hpp): build a CFG from an ARBOS function. The CFG provides an API which is compatible with the IKOS fixpoint algorithms but each basic block still contains AR statements. The ARBOS CFG is also augmented with useful information for dataflow analyses such as the set of used and defined variables.

* [include/analyzer/ar-wrapper/literal.hpp](include/analyzer/ar-wrapper/literal.hpp): class that converts an Arbos operand to an ARBOS-independent format.

* [include/analyzer/ar-wrapper/wrapper.hpp](include/analyzer/ar-wrapper/wrapper.hpp): implements an adaptor (namespace `ar`) for accessing to ARBOS AR. The purpose is to establish a layer of separation between ARBOS AR and all the analyses in [include/analyzer/analysis](include/analyzer/analysis).

* [include/analyzer/ar-wrapper/transformations.hpp](include/analyzer/ar-wrapper/transformations.hpp): implements an adaptor (namespace `transformations`) for modifying ARBOS AR with similar motivation than
above.

##### include/analyzer/checkers

This folder contains classes that check for properties on the code (out-of-bound accesses, divisions by zero, etc), given the result of an analysis.

##### include/analyzer/ikos-wrapper

* [include/analyzer/ikos-wrapper/iterators](include/analyzer/ikos-wrapper/iterators): wrapper to the IKOS fixpoints. It provides:
   * forward abstract interpreter
   * backward abstract interpreter
  The backward abstract interpreter is limited to dataflow analyses, otherwise although sound it will be too imprecise.

##### include/analyzer/domains

This folder contains analysis-specific abstract domains not available in ikos core library.

##### include/analyzer/utils

Common utilities for the analyses.

##### include/analyzer/examples

* [include/analyzer/examples/muaz.hpp](include/analyzer/examples/muaz.hpp): a micro language for semantic modelling of arrays and integer numbers.

#### scripts

* [scripts/ikos](scripts/ikos): ikos analyzer python script

#### python

* [python/ikos](python/ikos): ikos python module

#### src/ar-passes

* [src/ar-passes/analyzer.cpp](src/ar-passes/analyzer.cpp): Analyzer ARBOS pass. This is the entry point for all analyses.

* [src/ar-passes/ar_to_dot.cpp](src/ar-passes/ar_to_dot.cpp): ARBOS pass that translates AR code into dot format

* [src/ar-passes/inline_init_gv.cpp](src/ar-passes/inline_init_gv.cpp): Inline initialization of global variables to the main function.

* [src/ar-passes/pointer_shift_opt.cpp](src/ar-passes/pointer_shift_opt.cpp): Remove redundant arithmetic statements produced by the translation of LLVM getElementPtr instructions to ARBOS pointer shifts.
