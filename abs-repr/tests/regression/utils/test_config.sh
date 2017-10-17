#!/bin/bash

if ! command -v ikos-config >/dev/null 2>&1; then
    echo "error: could not find ikos-config" >&2
    exit 1
fi

export LLVM_PASSES_DIR="$(ikos-config --libdir)"
export ARBOS_PASSES_DIR="$LLVM_PASSES_DIR"
export PATH="$(ikos-config --bindir):$(ikos-config --llvm-bindir):$PATH"

if [ $(uname) = "Darwin" ]; then
    DYNLIB_EXT="dylib"
elif [ $(uname) = "Linux" ]; then
    DYNLIB_EXT="so"
else
    echo "Unsupported operating system: $(uname)"
    exit 1
fi

# Check existence of passes
llvm_arbos_module="llvm-to-ar.$DYNLIB_EXT"

[ -f "$LLVM_PASSES_DIR/$llvm_arbos_module" ] || { echo >&2 "$llvm_arbos_module not found.  Aborting."; exit 1; }

basic_parsing_ar_pass="libtest-parsing-basic.$DYNLIB_EXT"

[ -f "$ARBOS_PASSES_DIR/$basic_parsing_ar_pass" ] || { echo >&2 "$basic_parsing_ar_pass not found.  Aborting."; exit 1; }

LLVM_PASSES="-mem2reg -loweratomic -lowerswitch -instnamer"

PRE_ARBOS_PASSES=""

# Check existence of the executables
command -v clang >/dev/null 2>&1 || { echo >&2 "Requires clang but it's not installed.  Aborting."; exit 1; }
command -v opt >/dev/null 2>&1 || { echo >&2 "Requires opt but it's not installed.  Aborting."; exit 1; }
command -v arbos >/dev/null 2>&1 || { echo >&2 "Requires arbos but it's not installed.  Aborting."; exit 1; }
