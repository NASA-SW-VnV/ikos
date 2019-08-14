###############################################################################
#
# IKOS enums
#
# Author: Maxime Arthaud
#
# Contact: ikos@lists.nasa.gov
#
# Notices:
#
# Copyright (c) 2018-2019 United States Government as represented by the
# Administrator of the National Aeronautics and Space Administration.
# All Rights Reserved.
#
# Disclaimers:
#
# No Warranty: THE SUBJECT SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY OF
# ANY KIND, EITHER EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING, BUT NOT LIMITED
# TO, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL CONFORM TO SPECIFICATIONS,
# ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
# OR FREEDOM FROM INFRINGEMENT, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL BE
# ERROR FREE, OR ANY WARRANTY THAT DOCUMENTATION, IF PROVIDED, WILL CONFORM TO
# THE SUBJECT SOFTWARE. THIS AGREEMENT DOES NOT, IN ANY MANNER, CONSTITUTE AN
# ENDORSEMENT BY GOVERNMENT AGENCY OR ANY PRIOR RECIPIENT OF ANY RESULTS,
# RESULTING DESIGNS, HARDWARE, SOFTWARE PRODUCTS OR ANY OTHER APPLICATIONS
# RESULTING FROM USE OF THE SUBJECT SOFTWARE.  FURTHER, GOVERNMENT AGENCY
# DISCLAIMS ALL WARRANTIES AND LIABILITIES REGARDING THIRD-PARTY SOFTWARE,
# IF PRESENT IN THE ORIGINAL SOFTWARE, AND DISTRIBUTES IT "AS IS."
#
# Waiver and Indemnity:  RECIPIENT AGREES TO WAIVE ANY AND ALL CLAIMS AGAINST
# THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS, AS WELL
# AS ANY PRIOR RECIPIENT.  IF RECIPIENT'S USE OF THE SUBJECT SOFTWARE RESULTS
# IN ANY LIABILITIES, DEMANDS, DAMAGES, EXPENSES OR LOSSES ARISING FROM SUCH
# USE, INCLUDING ANY DAMAGES FROM PRODUCTS BASED ON, OR RESULTING FROM,
# RECIPIENT'S USE OF THE SUBJECT SOFTWARE, RECIPIENT SHALL INDEMNIFY AND HOLD
# HARMLESS THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS,
# AS WELL AS ANY PRIOR RECIPIENT, TO THE EXTENT PERMITTED BY LAW.
# RECIPIENT'S SOLE REMEDY FOR ANY SUCH MATTER SHALL BE THE IMMEDIATE,
# UNILATERAL TERMINATION OF THIS AGREEMENT.
#
###############################################################################


def auto(reset=None):
    ''' Helper for enumerations '''
    if reset is not None:
        auto.counter = reset
    value = auto.counter
    auto.counter += 1
    return value


class Result:
    OK = 0
    WARNING = 1
    ERROR = 2
    UNREACHABLE = 3

    STRING_LIST = ['ok', 'warning', 'error', 'unreachable']
    STRING_MAP = {
        'ok': OK,
        'safe': OK,
        'warning': WARNING,
        'error': ERROR,
        'unreachable': UNREACHABLE,
    }

    @classmethod
    def str(cls, v):
        return cls.STRING_LIST[v]

    @classmethod
    def from_str(cls, v):
        return cls.STRING_MAP[v]


class CheckKind:
    UNREACHABLE = auto(reset=0)
    UNEXPECTED_OPERAND = auto()
    UNINITIALIZED_VARIABLE = auto()
    ASSERT = auto()
    DIVISION_BY_ZERO = auto()
    SHIFT_COUNT = auto()
    _BEGIN_INT_OVERFLOW = auto()
    SIGNED_INT_UNDERFLOW = auto()
    SIGNED_INT_OVERFLOW = auto()
    UNSIGNED_INT_UNDERFLOW = auto()
    UNSIGNED_INT_OVERFLOW = auto()
    _END_INT_OVERFLOW = auto()
    NULL_POINTER_DEREF = auto()
    NULL_POINTER_COMPARISON = auto()
    INVALID_POINTER_COMPARISON = auto()
    POINTER_COMPARISON = auto()
    POINTER_OVERFLOW = auto()
    INVALID_POINTER_DEREF = auto()
    UNKNOWN_MEMORY_ACCESS = auto()
    UNALIGNED_POINTER = auto()
    _BEGIN_BUFFER_OVERFLOW = auto()
    BUFFER_OVERFLOW_GETS = auto()
    BUFFER_OVERFLOW = auto()
    _END_BUFFER_OVERFLOW = auto()
    _BEGIN_SOUNDNESS = auto()
    IGNORED_STORE = auto()
    IGNORED_MEMORY_COPY = auto()
    IGNORED_MEMORY_MOVE = auto()
    IGNORED_MEMORY_SET = auto()
    IGNORED_FREE = auto()
    IGNORED_CALL_SIDE_EFFECT_ON_POINTER_PARAM = auto()
    IGNORED_CALL_SIDE_EFFECT = auto()
    RECURSIVE_FUNCTION_CALL = auto()
    _END_SOUNDNESS = auto()
    _BEGIN_FUNCTION_CALL = auto()
    FUNCTION_CALL_INLINE_ASSEMBLY = auto()
    UNKNOWN_FUNCTION_CALL_POINTER = auto()
    FUNCTION_CALL = auto()
    _END_FUNCTION_CALL = auto()
    FREE = auto()

    SHORT_NAME_LIST = [
        'unreachable',
        'unexpected-operand',
        'uninitialized-variable',
        'assert',
        'division-by-zero',
        'shift-count',
        'begin-int-overflow',
        'signed-int-underflow',
        'signed-int-overflow',
        'unsigned-int-underflow',
        'unsigned-int-overflow',
        'end-int-overflow',
        'null-pointer-deref',
        'null-pointer-comparison',
        'invalid-pointer-comparison',
        'pointer-comparison',
        'pointer-overflow',
        'invalid-pointer-deref',
        'unknown-memory-access',
        'unaligned-pointer',
        'begin-buffer-overflow',
        'buffer-overflow-gets',
        'buffer-overflow',
        'end-buffer-overflow',
        'begin-soundness',
        'ignored-store',
        'ignored-memory-copy',
        'ignored-memory-move',
        'ignored-memory-set',
        'ignored-free',
        'ignored-call-side-effect-pointer-param',
        'ignored-call-side-effect',
        'recursive-function-call',
        'end-soundness',
        'begin-function-call',
        'function-call-inline-asm',
        'unknown-function-call-pointer',
        'function-call',
        'end-function-call',
        'free',
    ]

    @classmethod
    def short_name(cls, v):
        return cls.SHORT_NAME_LIST[v]

    LONG_NAME_LIST = [
        'dead code',
        'unexpected operand',
        'uninitialized variable',
        'assert',
        'division by zero',
        'shift count',
        'begin integer overflow',
        'signed integer underflow',
        'signed integer overflow',
        'unsigned integer underflow',
        'unsigned integer overflow',
        'end integer overflow',
        'null pointer dereference',
        'null pointer comparison',
        'invalid pointer comparison',
        'pointer comparison',
        'pointer overflow',
        'invalid pointer dereference',
        'unknown memory access',
        'unaligned pointer',
        'begin buffer overflow',
        'buffer overflow gets',
        'buffer overflow',
        'end buffer overflow',
        'begin soundness',
        'ignored store',
        'ignored memory copy',
        'ignored memory move',
        'ignored memory set',
        'ignored free',
        'ignored call side effect on pointer parameter',
        'ignored call side effect',
        'recursive function call',
        'end soundness',
        'begin function call',
        'function call inline assembly',
        'unknown function call pointer',
        'function call',
        'end function call',
        'free',
    ]

    @classmethod
    def long_name(cls, v):
        return cls.LONG_NAME_LIST[v]


class CheckerName:
    BUFFER_OVERFLOW = auto(reset=0)
    DIVISION_BY_ZERO = auto()
    NULL_POINTER_DEREF = auto()
    ASSERT_PROVER = auto()
    UNALIGNED_POINTER = auto()
    UNINITIALIZED_VARIABLE = auto()
    SIGNED_INT_OVERFLOW = auto()
    UNSIGNED_INT_OVERFLOW = auto()
    SHIFT_COUNT = auto()
    POINTER_OVERFLOW = auto()
    POINTER_COMPARE = auto()
    SOUNDNESS = auto()
    FUNCTION_CALL = auto()
    DEAD_CODE = auto()
    DOUBLE_FREE = auto()
    DEBUG = auto()
    MEMORY_WATCH = auto()

    SHORT_NAME_LIST = [
        'boa',
        'dbz',
        'nullity',
        'prover',
        'upa',
        'uva',
        'sio',
        'uio',
        'shc',
        'poa',
        'pcmp',
        'sound',
        'fca',
        'dca',
        'dfa',
        'dbg',
        'watch',
    ]
    SHORT_NAME_MAP = {v: k for k, v in enumerate(SHORT_NAME_LIST)}

    @classmethod
    def short_name(cls, v):
        return cls.SHORT_NAME_LIST[v]

    @classmethod
    def from_short_name(cls, v):
        return cls.SHORT_NAME_MAP[v]


class ValueKind:
    BEGIN_CONSTANT = auto(reset=0)
    UNDEFINED_CONSTANT = auto()
    INTEGER_CONSTANT = auto()
    FLOAT_CONSTANT = auto()
    NULL_CONSTANT = auto()
    STRUCT_CONSTANT = auto()
    BEGIN_SEQUENTIAL_CONSTANT = auto()
    ARRAY_CONSTANT = auto()
    VECTOR_CONSTANT = auto()
    END_SEQUENTIAL_CONSTANT = auto()
    AGGREGATE_ZERO_CONSTANT = auto()
    FUNCTION_POINTER_CONSTANT = auto()
    INLINE_ASSEMBLY_CONSTANT = auto()
    END_CONSTANT = auto()
    BEGIN_VARIABLE = auto()
    GLOBAL_VARIABLE = auto()
    LOCAL_VARIABLE = auto()
    INTERNAL_VARIABLE = auto()
    END_VARIABLE = auto()


class StatementKind:
    ASSIGNMENT = auto(reset=0)
    UNARY_OPERATION = auto()
    BINARY_OPERATION = auto()
    COMPARISON = auto()
    RETURN = auto()
    UNREACHABLE = auto()
    ALLOCATE = auto()
    POINTER_SHIFT = auto()
    LOAD = auto()
    STORE = auto()
    EXTRACT_ELEMENT = auto()
    INSERT_ELEMENT = auto()
    BEGIN_CALL_BASE = auto()
    CALL = auto()
    INVOKE = auto()
    END_CALL_BASE = auto()
    LANDING_PAD = auto()
    RESUME = auto()


class MemoryLocationKind:
    LOCAL = auto(reset=0)
    GLOBAL = auto()
    FUNCTION = auto()
    AGGREGATE = auto()
    ABSOLUTE_ZERO = auto()
    ARGV = auto()
    LIBC_ERRNO = auto()
    DYN_ALLOC = auto()


class FunctionCallCheckKind:
    NOT_FUNCTION = auto(reset=0)
    WRONG_SIGNATURE = auto()
    OK = auto()


class BufferOverflowCheckKind:
    FUNCTION = auto(reset=0)
    USE_AFTER_FREE = auto()
    USE_AFTER_RETURN = auto()
    HARDWARE_ADDRESSES = auto()
    OUT_OF_BOUND = auto()


class FilesTable:
    ID = auto(reset=0)
    PATH = auto()


class FunctionsTable:
    ID = auto(reset=0)
    NAME = auto()
    DEMANGLED = auto()
    DEFINITION = auto()
    FILE_ID = auto()
    LINE = auto()


class StatementsTable:
    ID = auto(reset=0)
    KIND = auto()
    FUNCTION_ID = auto()
    FILE_ID = auto()
    LINE = auto()
    COLUMN = auto()


class OperandsTable:
    ID = auto(reset=0)
    KIND = auto()
    REPR = auto()


class CallContextsTable:
    ID = auto(reset=0)
    CALL_ID = auto()
    FUNCTION_ID = auto()
    PARENT_ID = auto()


class MemoryLocationsTable:
    ID = auto(reset=0)
    KIND = auto()
    INFO = auto()


class ChecksTable:
    ID = auto(reset=0)
    KIND = auto()
    CHECKER = auto()
    STATUS = auto()
    STATEMENT_ID = auto()
    OPERANDS = auto()
    CALL_CONTEXT_ID = auto()
    INFO = auto()
