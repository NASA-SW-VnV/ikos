#IkosCC Version 1.0.0 Release Notes

This document contains the release notes for IkosCC, an abstract
interpretation-based static analyzer, release 1.0.0.

##Release Date

2015-04-14

##Description

This is an internal release towards our next public release. This
release finalizes the generic analysis framework that allows analyzing
**ARBOS AR** using standard abstract interpretation techniques.

This release contains the following analyses:

- `boa`: buffer overflow (out-of-bound array indexing)
- `dbz`: integer division by zero
- `nullity`: null pointer dereference
- `uva`: read of uninitialized integer variables
- `prover`: violation of user-defined properties to prove additional
runtime properties (similar to C `assert`)

Each of the analyses are implemented as **ARBOS plugins**.

##Changes

* Release 0.0.1 only implemented `boa`, the buffer overflow
  analysis. The implementation was not generic enough and therefore it
  had two different implementations for inter-procedural and
  intra-procedural modes.

  In release 1.0.0 the design has been significantly improved and as a
  result, one single implementation can execute the two modes.

* Release 1.0.0 adds four new analyses: `dbz`, `nullity`, `uva`, and
  `prover`.

* Release 0.0.1 used directly a numerical abstract domain (e.g.,
  intervals) in order to infer invariants. Therefore, it could not
  reason about pointer contents and it was unsound in presence of
  function pointers. Moreover, it did not keep track of nullity or
  whether used variables were initialized.

  Release 1.0.0 eliminates these limitations by implementing a value
  analysis (based on [1]) parameterized by a numerical abstract domain
  and augmented with nullity and initialization information.

* Release 1.0.0 performs an AR transformation called
  `pointer_shift_opt` that reduces the number of instructions
  generated during the translation from LLVM `getElementPtr` to AR
  `Pointer_Shift` instructions.

* Release 1.0.0 considers the initialization of all global variables
  while release 0.0.1 only considered global integer scalars.

* Release 1.0.0 provides a new tool called `IkosPP`, a LLVM bitecode
  optimizer. `IkosPP` performs several LLVM transformations to make
  faster and more precise the task of static analysis. The tool is a
  standalone application and its use is optional.
  
* Release 1.0.0 provides a python script `ikos_cc` that allows running
  all the analyses in a more portable way. Release 0.0.1 used a bash
  script with very limited portability.

* The installation and compilation process in release 1.0.0 has been
  highly simplified by replacing Makefile files and Bash scripts with
  the CMake system.

##References

1. Field-sensitive value analysis of embedded C programs with union
types and pointer arithmetics. Publised by Antoine Mine in
LCTES, 2006.

