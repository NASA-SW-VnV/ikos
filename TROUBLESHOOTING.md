Troubleshooting
===============

This document covers some common issues with IKOS, and how to solve them.

Contact
-------

ikos@lists.nasa.gov

Installation issues
-------------------

### "Could NOT find LLVM" while running cmake

CMake could not find LLVM.

First, install LLVM. This can usually be done with your package manager.

If this message still shows up, it means cmake cannot find the `llvm-config` command.

You can either add the LLVM binary directory in your PATH, or give cmake the full path to llvm-config, using `-DLLVM_CONFIG_EXECUTABLE=/path/to/llvm-config`

For instance, if you installed LLVM using Homebrew on Mac OS X, you can add LLVM in your path using:

```
$ PATH="$(brew --prefix)/opt/llvm/bin:$PATH"
```

### "Could NOT find Clang" while running cmake

CMake could not find Clang.

First, install Clang. This can usually be done with your package manager.

If this message still shows up, it means cmake cannot find the `clang` command.

You can either add the clang binary directory in your PATH, or give cmake the full path to clang, using `-DCLANG_EXECUTABLE=/path/to/clang`

### "Could not find ikos python module" while running ikos

The ikos command could not import the ikos python module.

The module should be under `/path/to/ikos-install/lib/python*/site-packages`

If the ikos python module is in another directory, make sure it is in your PYTHONPATH:

```
export PYTHONPATH=/path/to/ikos-python-module
```

### "Two passes with the same argument (-domtree) attempted to be registered!" while running ikos

You are probably trying to build IKOS with shared libraries (using `-DBUILD_SHARED_LIBS=ON`) and LLVM was linked statically (using `libLLVMxxx.a`).

Unfortunately, this doesn't work because LLVM uses global constructors to register command line options, and the global constructors end up being called twice.

Compiling IKOS with both `-DBUILD_SHARED_LIBS=ON` and `-DIKOS_LINK_LLVM_DYLIB=ON` should fix the issue by linking against the libLLVM shared library.

### "/usr/bin/ld: cannot find -lLLVMCore" while running Make

Your LLVM library was built as one single shared library `libLLVM.so` (`LLVM_BUILD_LLVM_DYLIB=1`), but CMake was configured to query specific library components and match link flags against them.

Compiling IKOS with both `-DBUILD_SHARED_LIBS=ON` and `-DIKOS_LINK_LLVM_DYLIB=ON` should fix the issue by linking against the single libLLVM shared library.

### `memory access violation at address: 0x00000088: no mapping at fault address` while running tests

This is a bug in the APRON library. Some OS distributions provide outdated versions of APRON. Building APRON from source (https://github.com/antoinemine/apron), instead of relying on a version made available via the OS' package repository, should fix the issue.

Analysis issues
---------------

### Exited with signal SIGKILL

IKOS probably ran out of memory.

See [Running ouf of memory](#running-out-of-memory)

### Running out of memory

IKOS might run out of memory on huge code bases.

Consider using the option `--no-fixpoint-cache`. It disables the cache of fixpoint for called functions, which decreases the memory usage at the cost of run time.

Known issues
------------

### Source Code Fortification

Source code fortification aims at making your source code more robust. It replaces regular `memset()`, `memcpy()` and `memmove()` calls to `__memset_chk()`, `__memcpy_chk()` and `__memmove_chk()`. According to Linux Standard Base Core Specification 4.1, the interfaces `__memset_chk()`, `__memcpy_chk()` and `__memmove_chk()` shall function in the same way as the interface `memset()`, `memcpy()` and `memmove()`, respectively, except that `__memset_chk()`, `__memcpy_chk()` and `__memmove_chk()` shall check for buffer overflow before computing a result. If an overflow is anticipated, the function shall abort and the program calling it shall exit.

The Buffer Overflow Analysis (BOA) in IKOS handles `__memset_chk()`, `__memcpy_chk()` and `__memmove_chk()` as unknown library functions, and won't report any warning. Consider using `-D_FORTIFY_SOURCE=0` when you compile your source code to LLVM bitcode manually.

### Analyzing multi-threaded code

IKOS does not handle analyzing multi-threaded code. Handling multi-threaded code in a sound static analyzer based on Abstract Interpretation (such as IKOS) is challenging and an ongoing research topic.
