Coding Standards
================

This document describes a few coding standards that are used in IKOS.

C++ Standard Version
--------------------

IKOS is currently written in C++14.

Source Code Formatting
----------------------

* Use 2 spaces for indentation ([Google Style Guide](https://google.github.io/styleguide/cppguide.html#Spaces_vs._Tabs))
* Do not indent inside a namespace ([LLVM Coding Standards](http://llvm.org/docs/CodingStandards.html#namespace-indentation))
* Indent each case in a switch statement ([Google Style Guide](https://google.github.io/styleguide/cppguide.html#Loops_and_Switch_Statements))
* No trailing whitespaces at the end of a line ([Google Style Guide](https://google.github.io/styleguide/cppguide.html#Horizontal_Whitespace))
* Remove unnecessary parenthesis around returned values ([Google Style Guide](https://google.github.io/styleguide/cppguide.html#Return_Values))
* Binary operators usually have spaces around them ([Google Style Guide](https://google.github.io/styleguide/cppguide.html#Horizontal_Whitespace))
* No extra space between a function name and arguments
* Do not align variable definitions ([Python Style Guide](https://www.python.org/dev/peps/pep-0008/#pet-peeves))
* Prefix member variables with an underscore (Arnaud's Style)
* Do not use `using namespace` in header files ([Google Style Guide](https://google.github.io/styleguide/cppguide.html#Namespaces), [LLVM Coding Standards](http://llvm.org/docs/CodingStandards.html#do-not-use-using-namespace-std))
* Sort included headers in the following order: standards, 3rd-party, our own
* Use Doxygen comments ([LLVM Coding Standards](http://llvm.org/docs/CodingStandards.html#doxygen-use-in-documentation-comments))
* Each line should fit within 80 columns ([LLVM Coding Standards](http://llvm.org/docs/CodingStandards.html#source-code-width))
* Treat compiler warnings like errors ([LLVM Coding Standards](http://llvm.org/docs/CodingStandards.html#treat-compiler-warnings-like-errors))
* Use `struct` only when all members are public, otherwise use `class` ([LLVM Coding Standards](http://llvm.org/docs/CodingStandards.html#use-of-class-and-struct-keywords))
* Use `auto` to make the code more readable ([LLVM Coding Standards](http://llvm.org/docs/CodingStandards.html#use-auto-type-deduction-to-make-code-more-readable))
* Use early exit and continue ([LLVM Coding Standards](http://llvm.org/docs/CodingStandards.html#use-early-exits-and-continue-to-simplify-code))
* Assert liberally ([LLVM Coding Standards](http://llvm.org/docs/CodingStandards.html#assert-liberally))
* Don't evaluate `end()` every time through a loop ([LLVM Coding Standards](http://llvm.org/docs/CodingStandards.html#don-t-evaluate-end-every-time-through-a-loop))
* Avoid `std::endl` ([LLVM Coding Standards](http://llvm.org/docs/CodingStandards.html#avoid-std-endl))
* Naming conventions are specified in the [.clang-tidy](../.clang-tidy) configuration file
* Use K&R style:
```c
if (cond) {
  f();
} else if (cond1) {
  g();
} else {
  h();
}
```

Automatic Formatting
--------------------

IKOS developers should use [clang-format](https://clang.llvm.org/docs/ClangFormat.html) to format the source code with a predefined set of style rules.

See [.clang-format](../.clang-format) for the set of rules.

Always run the following command before committing any changes:
```
$ git-clang-format -f
```

To run clang-format on the whole repository, use:
```
$ script/run-clang-format
```

Static Analysis
---------------

IKOS developers should use [clang-tidy](https://clang.llvm.org/extra/clang-tidy/) to perform diagnosis for typical programming errors, style violation, interface misuse, etc.

See [.clang-tidy](../.clang-tidy) for the set of checks.

To run clang-tidy on the whole repository, use:
```
$ mkdir build
$ cd build
$ cmake ..
$ ../script/run-clang-tidy
```

Dynamic Analysis
----------------

IKOS developers should use [clang sanitizers](https://clang.llvm.org/docs/UsersManual.html#controlling-code-generation) to check for undefined behaviors at run-time.

To build IKOS with clang sanitizers, use the following cmake options:
* `-DUSE_SANITIZER=Address` to use [Address Sanitizer](https://clang.llvm.org/docs/AddressSanitizer.html)
* `-DUSE_SANITIZER=Undefined` to use [Undefined Behavior Sanitizer](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html)
* `-DUSE_SANITIZER=Memory` to use [Memory Sanitizer](https://clang.llvm.org/docs/MemorySanitizer.html)
* `-DUSE_SANITIZER=Thread` to use [Thread Sanitizer](https://clang.llvm.org/docs/ThreadSanitizer.html)
* `-DUSE_SANITIZER=Leak` to use [Leak Sanitizer](https://clang.llvm.org/docs/LeakSanitizer.html)
