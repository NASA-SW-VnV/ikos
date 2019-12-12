IKOS version 3.0 release notes
==============================

Release date
------------

December 2019

List of changes
---------------

### IKOS Core changes

* Implemented a parallel fixpoint engine
* Implemented a partitioning abstract domain based on the values of a given integer variable
* Implemented a polymorphic memory abstract domain
* Fixed most abstract domains to be thread-safe
* Refactor the pointer and memory abstract domain interfaces

### LLVM frontend changes

* Upgraded LLVM from 8.0.x to 9.0.x

### Analyzer changes

* Implemented a parallel interprocedural and intraprocedural value analysis
* Add an option to enable abstract domain partitioning based on the returned value of a function


IKOS version 2.2 release notes
==============================

Release date
------------

August 2019

List of changes
---------------

### IKOS Core changes

* Fixed non-convergence issues in the fixpoint iterator
* Improved the precision of the modulo operator for the DBM domain
* Improved the weak topological order implementation
* Implemented the narrowing operator with a threshold

### Analyzer changes

* Improved the analysis and checks of the standard library functions
* Implemented a logger showing the progress of the analysis
* Added intrinsics to model library functions
* Added a checker to watch memory writes at a given location
* Added a checker to warn about unsound statements
* Added checks for recursive calls
* Forward `-I`, `-D`, `-W`, `-w` and `-m` flags to clang
* Support the analysis of LLVM text assembly file `.ll`
* Added an option to disable the cache of fixpoints
* Added options to control the fixpoint engine

### LLVM frontend changes

* Upgraded LLVM from 7.0.x to 8.0.x
* Improved the support of LLVM debug information
* Improved the error message for mismatch of intrinsics
* Added support for empty array fields in structures

### CMake changes

* Added support for linking with LLVM shared libraries
* Added version detection for GMP, MPFR, PPL, SQLite

### Overall changes

* Added several sections to the documentation:
   - Analysis assumptions
   - Modeling library functions
   - Analysis of embedded software


IKOS version 2.1 release notes
==============================

Release date
------------

December 2018

List of changes
---------------

### IKOS Core changes

* Fixed a non-convergence issue in the fixpoint iterator

### LLVM frontend changes

* Upgraded LLVM from 4.0.x to 7.0.x

### Ikos-Scan changes

* Implemented ikos-scan, a tool to analyze a whole C/C++ project using IKOS

### Overall changes

* Added support for Windows using the MinGW compiler


IKOS version 2.0 release notes
==============================

Release date
------------

October 2018

List of changes
---------------

### IKOS Core changes

* Updated the directory structure:
   - Renamed `algorithms` to `adt`;
   - Renamed `iterator` to `fixpoint`;
   - Added subdirectories under `domain` and `value` for each group of domains;
   - Added a `legacy` folder with unmaintained code. It will be removed in the future.
* Improved abstract domain interfaces using CRTP. Removed all the abstract domain traits.
* Added generic traits:
   - `DumpableTraits` to dump an object on a stream, for debugging purpose;
   - `IndexableTraits` to get a unique index representing an object;
   - `GraphTraits` to traverse a graph. It replaces the previous control flow graph API.
* Added a faster implementation of Patricia trees.
* Added machine integer abstract domains.
* Added unit tests using Boost.Test.
* Implemented a new pointer constraints system solver, previously called `pta`.
* Implemented `machine_int::PolymorphicDomain`, a machine integer abstract domain using runtime polymorphism that allows to use different abstract domains at runtime.
* Implemented `machine_int::NumericDomainAdapter`, a machine integer abstract domain wrapping a numeric abstract domain.
* Added abstract domain operators to perform widening with a threshold.
* Added a flow-sensitive context-sensitive pointer analysis in `ValueDomain`.

### Analyzer changes

* Moved most of the implementation from the header files into the `src` directory.
* Remove the `--ikos-pp` option. It is recommended not to use optimizations.
* Added the `--domain` option, to choose the numerical abstract domain at runtime.
* Added the `--hardware-addresses` option, for software using Direct Memory Access (DMA).
* Added the `--argc` option, to specify the value of `argc` during the analysis.
* Added support for global variable dynamic initialization.
* Added support for the `volatile` keyword.
* Fix exception propagation analysis for C++.
* Added `ikos/analyzer/intrinsic.h`, a header file with IKOS intrinsics.
* Improved database writes by creating a new transaction every 8192 queries.
* Updated the output database schema to store more information about the source code.
* Improved the uninitialized variable analysis.
* Added a pass to find widening hints.
* Improved warning and error messages using LLVM debug information.
* Implemented several checks:
   - signed and unsigned integer overflow (sio, uio);
   - invalid shift count (shc);
   - pointer overflow (poa);
   - invalid pointer comparison (pcmp);
   - invalid function call (fca);
   - dead code (dca);
   - double free and lifetime analysis (dfa);
   - unsound analysis (sound).

### AR changes

* Implemented a new Abstract Representation, based on the design of LLVM. The main changes are:
   - Improved memory management using clear ownership. The context owns types, a bundle owns functions, etc;
   - Added `isa<>`, `cast<>` and `dyn_cast<>` utilities, à la LLVM;
   - Added integer signedness information in the type system;
   - Added overflow and wrapping behaviours for integer operations;
   - Added visitors for statements and values;
   - Added intrinsics functions;
   - Added support for the `volatile` attribute;
   - Added traceability utilities, allowing to attach debug information to an AR object.
* Added a static type checker.
* Removed the branching-opt pass and added the simplify-cfg pass, a simpler version.
* Added the simplify upcast comparison pass.

### LLVM frontend changes

* Added 3 different optimization levels in ikos-pp (none, basic and aggressive)
* Implemented a new translation from LLVM to Abstract Representation (AR). The main changes are:
   - In memory translation instead of writing into a file;
   - Attach LLVM debug information to AR;
   - Recover integer signedness information from debug information and several heuristics.
* Added the ikos-import tool. It translates LLVM bitcode into AR, useful for debugging.
* Added regression tests for the new translation from LLVM to AR.

### CMake changes

* Add a target `check` that builds and runs all the tests.

### Ikos-View changes

* Added ikos-view, a web interface to examine IKOS results.

### Overall changes

* Updated the naming convention.


IKOS version 1.3 release notes
==============================

Release date
------------

October 2017

List of changes
---------------

### IKOS Core changes

* Added an implementation of the Gauge domain.
* Combined the memory domain and the pointer domain, improving the precision for arrays of pointers and virtual tables.
* Improved the documentation

### Analyzer changes

* Added a report generator. The analyzer can now export the results in a gcc-style format, json or csv.
* Added an analysis of unaligned pointers.
* Improved the analysis performance by keeping previous fixpoints of called functions in memory.
* Added the intrinsic function `__ikos_debug()`, that dumps the current invariant.
* Added support for analyzing variable argument functions (`va_start`, `va_arg`, `va_end` and `va_copy`).
* Added support for analyzing structures in register (`insertelement` and `extractelement` in LLVM).
* Fixed an unsoundness in the exception analysis.
* Added a `settings` and `times` table in the result database, allowing a user to query the analysis options and timing results.
* Added special checks that warn about dangerous code patterns (such as casts from integers to pointers).
* Fixed and improved the analysis of bitwise and conversion operators.
* Moved all global variable initializations into the internal function `__ikos_init_globals`, also fixing the initialization of global objects in C++.

### ARBOS changes

* Added support for C variable-length arrays (VLA).

### LLVM frontend changes

* Upgraded to LLVM 4.0.x


IKOS version 1.2 release notes
==============================

Release date
------------

October 2016

List of changes
---------------

### IKOS Core changes

* Added an abstract domain to handle C++ exceptions.

### Analyzer changes

* Implemented a proper C++ exception propagation handling using abstract interpretation. IKOS correctly handles C++ code with exceptions.
* Improved the analysis of string-related functions (such as strlen, strcpy and strcat).
* Fixed the precision of the null dereference analysis.

### ARBOS changes

* Removed AR_Unwind as it is no longer generated by LLVM since LLVM 3.1
* Added APIs to set and retrieve the "resume" basic block in an AR_Code.


IKOS version 1.1.2 release notes
================================

Release date
------------

September 2016

List of changes
---------------

### IKOS Core changes

* Added interfaces for abstract domains, nullity domains, uninitialized variables domains, pointer domains and memory domains.
* Moved the pointer domain, the value domain and the summary domain under IKOS core.

### Analyzer changes

* Added a pass to unify exit nodes.

### ARBOS changes

* Added APIs to set and retrieve "unreachable" and "unwind" basic blocks in an AR_Code


IKOS version 1.1.1 release notes
================================

Release date
------------

August 2016

List of changes
---------------

### Analyzer changes

* Added a pass `branching-opt` to optimize all Control Flow Graphs of the AR, before the analysis. The pass detects unreachable paths in the graphs and removes them.

### IKOS Core changes

* Improved the precision of the interval domain on most binary operators.
* Added a wrapper for APRON.

### Other changes

* Improved the bootstrap script. It is now able to build and install zlib, ncurses, libedit, GMP, SQLite and Python.
* Added installation instructions for Arch Linux, CentOS, Debian, Fedora, Mac OS X, Red Hat and Ubuntu.
* Added support for gcc 4.9.2 and boost 1.55.0.


IKOS version 1.1.0 release notes
================================

Release date
------------

June 2016

List of changes
---------------

### Analyzer changes

* Added the ability to demangle C++ function names.
* We now handle `calloc()` correctly. In IKOS 1.0.0, the function call was just ignored.
* Added the runtime options `--display-invariants` and `--display-checks`.
* Added a column called `column` to the result tables in the output database, containing the column number in the source code.
* Added a column called `stmt_uid` to the result tables in the output database, containing the UID of the checked statement.
* Performance improvement (the analysis is 70% faster in average).
* Bug fixes.

### LLVM Frontend changes

The LLVM frontend now supports both LLVM 3.7 and 3.8.

### ARBOS changes

ARBOS now gives the column number in addition to the line number in `AR_Source_Location`.


IKOS version 1.0.0 release notes
================================

Release date
------------

May 2016

List of changes
---------------

### LLVM Frontend changes

In this release we upgraded our LLVM frontend from version 2.9 to 3.7.

To have a C/C++ program fully represented in AR, there are several code transformation we had to handle with the frontend's intermediate representation. When using the LLVM framework as the frontend, we transform the following LLVM instructions so they can be expressed in AR:

* `phi` instruction
* `getelementptr` instruction
* `insertvalue` and `extractvalue` instructions
* `i/fcmp` and `select` instructions
* removed `br`
* `constantexpr` and its subclasses
* translation of intrinsic calls to AR-supported intrinsic calls. We currently support `memset`, `memcpy`, `memmove`, `vastart`, `vaend`, `va_arg`, and `vacopy`.

In the previous release v0.0.1 we handled all transformation in **ARBOS**. The previous tool chain architecture defined another intermediate representation, called **AIR**, that served as a direct translation from the LLVM bitcode to an S-expression format that was fed into **ARBOS**. **ARBOS** then parsed and transformed **AIR** to **AR** in memory.

This release removes **AIR** entirely. All transformation are done in **LLVMAR**, while **ABROS** only parses the AR and makes it available in memory for ARBOS analysis plugins to traverse and perform analysis on it.

### ARBOS API changes

* Each `AR_Global_Variable` may contain an initializer represented as `AR_Code`. In the previous release we dedicated an initializer function for each global variable. These initializer functions were individually invoked in `main()`. In this release, analyzers need to dive into the initializer of an `AR_Global_Variable` to analyze the initializer code.
* Differentiates between store vs. real sizes of an `AR_Type`.
* Supports `varags` intrinsic calls in the AR. This was not supported in the previous release.
* Function pointers are now available in the AR model; they were misinterpreted and were not available in the previous release.
