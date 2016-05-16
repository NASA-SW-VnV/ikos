#!/bin/bash

: "${LLVM_INSTALL}?Need to set LLVM_INSTALL}"
: "${IKOS_INSTALL}:?Need to set IKOS_INSTALL non-empty}"

export LLVM_PASSES_DIR="$IKOS_INSTALL/lib"
export ARBOS_PASSES_DIR="$IKOS_INSTALL/lib"
export PATH="$LLVM_INSTALL/bin:$IKOS_INSTALL/bin:$PATH"

if [ $(uname) = "Darwin" ]; then
    export DYLD_LIBRARY_PATH="$IKOS_INSTALL/lib:$DYLD_LIBRARY_PATH"
    DYNLIB_EXT="dylib"
elif [ $(uname) = "Linux" ]; then
    export LD_LIBRARY_PATH="$IKOS_INSTALL/lib:$LD_LIBRARY_PATH"
    DYNLIB_EXT="so"
else
    echo "Unsupported operating system: $(uname)"
    exit 1
fi

llvm_arbos_module="llvm-to-ar.$DYNLIB_EXT"

[ -f "$ARBOS_PASSES_DIR/$llvm_arbos_module" ] || { echo >&2 "$llvm_arbos_module not found.  Aborting."; exit 1; }

basic_parsing_ar_pass="libtest-parsing-basic.$DYNLIB_EXT"

[ -f "$ARBOS_PASSES_DIR/$basic_parsing_ar_pass" ] || { echo >&2 "$basic_parsing_ar_pass not found.  Aborting."; exit 1; }

LLVM_PASSES="-mem2reg -loweratomic -lowerswitch -instnamer"
PRE_ARBOS_PASSES=""

# Check existence of the executables
command -v clang >/dev/null 2>&1 || { echo >&2 "Requires clang but it's not installed.  Aborting."; exit 1; }
command -v opt >/dev/null 2>&1 || { echo >&2 "Requires opt but it's not installed.  Aborting."; exit 1; }
command -v arbos >/dev/null 2>&1 || { echo >&2 "Requires arbos but it's not installed.  Aborting."; exit 1; }

# Check existence of passes
[ -f "$LLVM_PASSES_DIR/$llvm_arbos_module" ] || { echo >&2 "$llvm_arbos_module not found.  Aborting."; exit 1; }
