IKOS VERSION 1.1.0 RELEASE NOTES
================================


RELEASE DATE
------------

June 2016


LIST OF CHANGES
---------------

### Analyzer Changes

* Add the ability to demangle C++ function names
* Handle `calloc()` correctly. In IKOS 1.0.0, the function call was just ignored.
* Add the runtime options --display-invariants and --display-checks
* Add a column called `column` to the result tables in the output database, containing the column number in the source code.
* Add a column called `stmt_uid` to the result tables in the output database, containing the UID of the checked statement.
* Performance improvement (the analysis is 70% faster in average)
* Bug fixes

### LLVM Frontend Changes

The LLVM frontend now supports both LLVM 3.7 and 3.8.

### ARBOS Changes

ARBOS now gives the column number in addition to the line number in AR_Source_Location.


IKOS VERSION 1.0.0 RELEASE NOTES
================================


RELEASE DATE
------------

May 2016

LIST OF CHANGES
---------------

### LLVM Frontend Changes

In this release we upgraded our LLVM frontend from version 2.9 to 3.7.

To have a C/C++ program fully represented in AR, there are several code transformation we had to handle with the frontend's intermediate representation. When using the LLVM framework as the frontend, we transform the following LLVM instructions so they can be expressed in AR:

* 'phi' instruction
* 'getelementptr' instruction
* 'insertvalue' and 'extractvalue' instructions
* 'i/fcmp' and 'select' instructions
* removed 'br'
* 'constantexpr' and its subclasses
* translation of intrinsic calls to AR-supported intrinsic calls. We currently support memset, memcpy, memmove, vastart, vaend, va_arg, and vacopy.

In the previous release v0.0.1 we handled all transformation in **ARBOS**. The previous tool chain architecture defined another intermediate representation, called **AIR**, that served as a direct translation from the LLVM bitcode to an S-expression format that was fed into **ARBOS**. **ARBOS** then parsed and transformed **AIR** to **AR** in memory.

This release removes **AIR** entirely. All transformation are done in **LLVMAR**, while **ABROS** only parses the AR and makes it available in memory for ARBOS analysis plugins to traverse and perform analysis on it.

### ARBOS API Changes

* Each AR_Global_Variable may contain an initializer represented as AR_Code. In the previous release we dedicated an initializer function for each global variable. These initializer functions were individually invoked in main(). In this release, analyzers need to dive into the initializer of an AR_Global_Variable to analyze the initializer code.
* Differentiates between store vs. real sizes of an AR_Type.
* Supports 'varags' intrinsic calls in the AR. This was not supported in the previous release
* Function pointers are now available in the AR model; they were misinterpreted and were not available in the previous release


KNOWN ISSUES
============

Source Code Fortification
-------------------------

Source code fortification aims at making your source code more robust. It replaces regular memset(), memcpy() and memmove() calls to __memset_chk(), __memcpy_chk() and __memmove_chk(). According to Linux Standard Base Core Specification 4.1, the interfaces __memset_chk(), __memcpy_chk() and __memmove_chk() shall function in the same way as the interface memset(), memcpy() and memmove(), respectively, except that __memset_chk(), __memcpy_chk() and __memmove_chk() shall check for buffer overflow before computing a result. If an overflow is anticipated, the function shall abort and the program calling it shall exit.

The Buffer Overflow Analysis (BOA) in IKOS handles __memset_chk(), __memcpy_chk() and __memmove_chk() as unknown library functions, and won't report any warning. Consider using -D_FORTIFY_SOURCE=0 when you compile your source code to LLVM bitcode manually.

Handling Global Variables
-------------------------

Prior to performing any analysis, the initialization code of the global variables in the AR program model are inlined in function **main** if available. This transformation of the AR program model is implemented as an AR pass located at **analyzer/src/ar-passes/inline-init-gv.cpp**.

LLVM Frontend Limitation
------------------------

The current implementation of LLVM frontend does not support LLVM vector type (http://llvm.org/docs/LangRef.html#vector-type).

Exception Handling
------------------

IKOS 1.0.0 is able to analyze C++ code containing exceptions, but exception propagation through functions is not handled. If the code you are analyzing can raise an exception and does not catch it within the same function, IKOS might be unsound, meaning that it can miss runtime errors. If your code only uses exceptions to report a runtime error and stop the program, then IKOS should be sound.

Octagon Abstract Domain
-----------------------

The octagon abstract domain has some implementation issues. Do not use it.

Functions Returning a Temporary Array/Structure
-----------------------------------------------

IKOS is unsound if you have a function that returns a temporary array or structure. For instance:

```
int* f() {
  int tab[42];
  return tab;
}
```

By the way, this should be forbidden and your compiler should warn you.

Variable Length Array
---------------------

IKOS is unsound if you have a variable length array in your code. A variable length array, or [VLA](https://gcc.gnu.org/onlinedocs/gcc/Variable-Length.html), is a stack allocated array with a dynamic size. For instance:

```
void f(int size) {
  int tab[size]; // VLA, allowed in ISO C99
  ...
}
```
