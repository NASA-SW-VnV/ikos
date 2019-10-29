/*******************************************************************************
 *
 * \file
 * \brief Implementation of Intrinsic
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2017-2019 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Disclaimers:
 *
 * No Warranty: THE SUBJECT SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY OF
 * ANY KIND, EITHER EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL CONFORM TO SPECIFICATIONS,
 * ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 * OR FREEDOM FROM INFRINGEMENT, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL BE
 * ERROR FREE, OR ANY WARRANTY THAT DOCUMENTATION, IF PROVIDED, WILL CONFORM TO
 * THE SUBJECT SOFTWARE. THIS AGREEMENT DOES NOT, IN ANY MANNER, CONSTITUTE AN
 * ENDORSEMENT BY GOVERNMENT AGENCY OR ANY PRIOR RECIPIENT OF ANY RESULTS,
 * RESULTING DESIGNS, HARDWARE, SOFTWARE PRODUCTS OR ANY OTHER APPLICATIONS
 * RESULTING FROM USE OF THE SUBJECT SOFTWARE.  FURTHER, GOVERNMENT AGENCY
 * DISCLAIMS ALL WARRANTIES AND LIABILITIES REGARDING THIRD-PARTY SOFTWARE,
 * IF PRESENT IN THE ORIGINAL SOFTWARE, AND DISTRIBUTES IT "AS IS."
 *
 * Waiver and Indemnity:  RECIPIENT AGREES TO WAIVE ANY AND ALL CLAIMS AGAINST
 * THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS, AS WELL
 * AS ANY PRIOR RECIPIENT.  IF RECIPIENT'S USE OF THE SUBJECT SOFTWARE RESULTS
 * IN ANY LIABILITIES, DEMANDS, DAMAGES, EXPENSES OR LOSSES ARISING FROM SUCH
 * USE, INCLUDING ANY DAMAGES FROM PRODUCTS BASED ON, OR RESULTING FROM,
 * RECIPIENT'S USE OF THE SUBJECT SOFTWARE, RECIPIENT SHALL INDEMNIFY AND HOLD
 * HARMLESS THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS,
 * AS WELL AS ANY PRIOR RECIPIENT, TO THE EXTENT PERMITTED BY LAW.
 * RECIPIENT'S SOLE REMEDY FOR ANY SUCH MATTER SHALL BE THE IMMEDIATE,
 * UNILATERAL TERMINATION OF THIS AGREEMENT.
 *
 ******************************************************************************/

#include <ikos/ar/semantic/bundle.hpp>
#include <ikos/ar/semantic/intrinsic.hpp>
#include <ikos/ar/semantic/type.hpp>
#include <ikos/ar/semantic/type_visitor.hpp>
#include <ikos/ar/support/assert.hpp>

namespace ikos {
namespace ar {

FunctionType* Intrinsic::type(Bundle* bundle, ID id) {
  return type(bundle, id, /* template_ty = */ nullptr);
}

FunctionType* Intrinsic::type(Bundle* bundle, ID id, Type* template_ty) {
  ikos_assert(id != NotIntrinsic);

  // Helpers
  Context& ctx = bundle->context();
  VoidType* void_ty = VoidType::get(ctx);
  IntegerType* ui1_ty = IntegerType::ui1(ctx);
  IntegerType* si8_ty = IntegerType::si8(ctx);
  IntegerType* ui32_ty = IntegerType::ui32(ctx);
  IntegerType* si32_ty = IntegerType::si32(ctx);
  IntegerType* size_ty = IntegerType::size_type(bundle);
  IntegerType* ssize_ty = IntegerType::ssize_type(bundle);
  PointerType* void_ptr_ty = PointerType::get(ctx, si8_ty);
  PointerType* char_ptr_ty = PointerType::get(ctx, si8_ty);
  PointerType* file_ptr_ty =
      PointerType::get(ctx, OpaqueType::libc_file_type(ctx));

  Type* ret_ty = nullptr;
  FunctionType::ParamTypes params;
  bool var_arg = false;

  switch (id) {
    case MemoryCopy: {
      ret_ty = void_ty;              // ret
      params.push_back(void_ptr_ty); // dest
      params.push_back(void_ptr_ty); // src
      params.push_back(size_ty);     // length
      params.push_back(ui32_ty);     // dest_alignment
      params.push_back(ui32_ty);     // src_alignment
      params.push_back(ui1_ty);      // volatile
    } break;
    case MemoryMove: {
      ret_ty = void_ty;              // ret
      params.push_back(void_ptr_ty); // dest
      params.push_back(void_ptr_ty); // src
      params.push_back(size_ty);     // length
      params.push_back(ui32_ty);     // dest_alignment
      params.push_back(ui32_ty);     // src_alignment
      params.push_back(ui1_ty);      // volatile
    } break;
    case MemorySet: {
      ret_ty = void_ty;              // ret
      params.push_back(void_ptr_ty); // ptr
      params.push_back(si8_ty);      // value
      params.push_back(size_ty);     // length
      params.push_back(ui32_ty);     // alignment
      params.push_back(ui1_ty);      // volatile
    } break;
    case VarArgStart: {
      ret_ty = void_ty;              // ret
      params.push_back(void_ptr_ty); // va_list
    } break;
    case VarArgEnd: {
      ret_ty = void_ty;              // ret
      params.push_back(void_ptr_ty); // va_list
    } break;
    case VarArgGet: {
      // Suppose that va_arg always returns a void*
      ret_ty = void_ptr_ty;          // ret
      params.push_back(void_ptr_ty); // va_list
    } break;
    case VarArgCopy: {
      ret_ty = void_ty;              // ret
      params.push_back(void_ptr_ty); // dest
      params.push_back(void_ptr_ty); // src
    } break;
    case StackSave: {
      ret_ty = void_ptr_ty; // ret
    } break;
    case StackRestore: {
      ret_ty = void_ty;              // ret
      params.push_back(void_ptr_ty); // stack ptr
    } break;
    case LifetimeStart: {
      ret_ty = void_ty;              // ret
      params.push_back(size_ty);     // size
      params.push_back(void_ptr_ty); // ptr
    } break;
    case LifetimeEnd: {
      ret_ty = void_ty;              // ret
      params.push_back(size_ty);     // size
      params.push_back(void_ptr_ty); // ptr
    } break;
    case EhTypeidFor: {
      ret_ty = si32_ty;              // ret
      params.push_back(void_ptr_ty); // exception ptr
    } break;
    case Trap: {
      ret_ty = void_ty; // ret
    } break;
    // <ikos/analyzer/intrinsic.h>
    case IkosAssert: {
      ret_ty = void_ty;          // ret
      params.push_back(ui32_ty); // condition
    } break;
    case IkosAssume: {
      ret_ty = void_ty;          // ret
      params.push_back(ui32_ty); // condition
    } break;
    case IkosNonDet: {
      ikos_assert(template_ty != nullptr);
      ret_ty = template_ty; // ret
    } break;
    case IkosCounterInit: {
      ret_ty = size_ty;          // ret
      params.push_back(size_ty); // initial value
    } break;
    case IkosCounterIncr: {
      ret_ty = size_ty;          // ret
      params.push_back(size_ty); // counter
      params.push_back(size_ty); // increment
    } break;
    case IkosCheckMemAccess: {
      ret_ty = void_ty;              // ret
      params.push_back(void_ptr_ty); // ptr
      params.push_back(size_ty);     // size
    } break;
    case IkosCheckStringAccess: {
      ret_ty = void_ty;              // ret
      params.push_back(char_ptr_ty); // str
    } break;
    case IkosAssumeMemSize: {
      ret_ty = void_ty;              // ret
      params.push_back(void_ptr_ty); // ptr
      params.push_back(size_ty);     // size
    } break;
    case IkosForgetMemory: {
      ret_ty = void_ty;              // ret
      params.push_back(void_ptr_ty); // ptr
      params.push_back(size_ty);     // size
    } break;
    case IkosAbstractMemory: {
      ret_ty = void_ty;              // ret
      params.push_back(void_ptr_ty); // ptr
      params.push_back(size_ty);     // size
    } break;
    case IkosWatchMemory: {
      ret_ty = void_ty;              // ret
      params.push_back(void_ptr_ty); // ptr
      params.push_back(size_ty);     // size
    } break;
    case IkosPartitioningVar: {
      ikos_assert(template_ty != nullptr);
      ret_ty = void_ty;              // ret
      params.push_back(template_ty); // x
    } break;
    case IkosPartitioningJoin: {
      ret_ty = void_ty; // ret
    } break;
    case IkosPartitioningDisable: {
      ret_ty = void_ty; // ret
    } break;
    case IkosPrintInvariant: {
      ret_ty = void_ty; // ret
    } break;
    case IkosPrintValues: {
      ret_ty = void_ty;              // ret
      params.push_back(char_ptr_ty); // dest
      var_arg = true;
    } break;
    // <stdlib.h>
    case LibcMalloc: {
      ret_ty = void_ptr_ty;      // ret
      params.push_back(size_ty); // size
    } break;
    case LibcCalloc: {
      ret_ty = void_ptr_ty;      // ret
      params.push_back(size_ty); // count
      params.push_back(size_ty); // size
    } break;
    case LibcValloc: {
      ret_ty = void_ptr_ty;      // ret
      params.push_back(size_ty); // size
    } break;
    case LibcAlignedAlloc: {
      ret_ty = void_ptr_ty;      // ret
      params.push_back(size_ty); // alignment
      params.push_back(size_ty); // size
    } break;
    case LibcRealloc: {
      ret_ty = void_ptr_ty;          // ret
      params.push_back(void_ptr_ty); // ptr
      params.push_back(size_ty);     // size
    } break;
    case LibcFree: {
      ret_ty = void_ty;              // ret
      params.push_back(void_ptr_ty); // ptr
    } break;
    case LibcAbs: {
      ret_ty = si32_ty;          // ret
      params.push_back(si32_ty); // i
    } break;
    case LibcRand: {
      ret_ty = si32_ty; // ret
    } break;
    case LibcSrand: {
      ret_ty = void_ty;          // ret
      params.push_back(ui32_ty); // seed
    } break;
    case LibcExit: {
      ret_ty = void_ty;          // ret
      params.push_back(si32_ty); // status
    } break;
    case LibcAbort: {
      ret_ty = void_ty; // ret
    } break;
    // <errno.h>
    case LibcErrnoLocation: {
      ret_ty = PointerType::get(ctx, si32_ty); // ret
    } break;
    // <fcntl.h>
    case LibcOpen: {
      ret_ty = si32_ty;              // ret
      params.push_back(char_ptr_ty); // path
      params.push_back(si32_ty);     // oflags
      var_arg = true;
    } break;
    // <unistd.h>
    case LibcClose: {
      ret_ty = si32_ty;          // ret
      params.push_back(si32_ty); // fildes
    } break;
    case LibcRead: {
      ret_ty = ssize_ty;             // ret
      params.push_back(si32_ty);     // fildes
      params.push_back(void_ptr_ty); // buffer
      params.push_back(size_ty);     // nbytes
    } break;
    case LibcWrite: {
      ret_ty = ssize_ty;             // ret
      params.push_back(si32_ty);     // fildes
      params.push_back(void_ptr_ty); // buffer
      params.push_back(size_ty);     // nbytes
    } break;
    // <stdio.h>
    case LibcGets: {
      ret_ty = char_ptr_ty;          // ret
      params.push_back(char_ptr_ty); // str
    } break;
    case LibcFgets: {
      ret_ty = char_ptr_ty;          // ret
      params.push_back(char_ptr_ty); // str
      params.push_back(si32_ty);     // size
      params.push_back(file_ptr_ty); // stream
    } break;
    case LibcGetc: {
      ret_ty = si32_ty;              // ret
      params.push_back(file_ptr_ty); // stream
    } break;
    case LibcFgetc: {
      ret_ty = si32_ty;              // ret
      params.push_back(file_ptr_ty); // stream
    } break;
    case LibcGetchar: {
      ret_ty = si32_ty; // ret
    } break;
    case LibcPuts: {
      ret_ty = si32_ty;              // ret
      params.push_back(char_ptr_ty); // s
    } break;
    case LibcFputs: {
      ret_ty = si32_ty;              // ret
      params.push_back(char_ptr_ty); // s
      params.push_back(file_ptr_ty); // stream
    } break;
    case LibcPutc: {
      ret_ty = si32_ty;              // ret
      params.push_back(si32_ty);     // c
      params.push_back(file_ptr_ty); // stream
    } break;
    case LibcFputc: {
      ret_ty = si32_ty;              // ret
      params.push_back(si32_ty);     // c
      params.push_back(file_ptr_ty); // stream
    } break;
    case LibcPrintf: {
      ret_ty = si32_ty;              // ret
      params.push_back(char_ptr_ty); // format
      var_arg = true;
    } break;
    case LibcFprintf: {
      ret_ty = si32_ty;              // ret
      params.push_back(file_ptr_ty); // stream
      params.push_back(char_ptr_ty); // format
      var_arg = true;
    } break;
    case LibcSprintf: {
      ret_ty = si32_ty;              // ret
      params.push_back(char_ptr_ty); // buffer
      params.push_back(char_ptr_ty); // format
      var_arg = true;
    } break;
    case LibcSnprintf: {
      ret_ty = si32_ty;              // ret
      params.push_back(char_ptr_ty); // buffer
      params.push_back(size_ty);     // size
      params.push_back(char_ptr_ty); // format
      var_arg = true;
    } break;
    case LibcScanf: {
      ret_ty = si32_ty;              // ret
      params.push_back(char_ptr_ty); // format
      var_arg = true;
    } break;
    case LibcFscanf: {
      ret_ty = si32_ty;              // ret
      params.push_back(file_ptr_ty); // stream
      params.push_back(char_ptr_ty); // format
      var_arg = true;
    } break;
    case LibcSscanf: {
      ret_ty = si32_ty;              // ret
      params.push_back(char_ptr_ty); // buffer
      params.push_back(char_ptr_ty); // format
      var_arg = true;
    } break;
    case LibcFopen: {
      ret_ty = file_ptr_ty;          // ret
      params.push_back(char_ptr_ty); // filename
      params.push_back(char_ptr_ty); // mode
    } break;
    case LibcFclose: {
      ret_ty = si32_ty;              // ret
      params.push_back(file_ptr_ty); // stream
    } break;
    case LibcFflush: {
      ret_ty = si32_ty;              // ret
      params.push_back(file_ptr_ty); // stream
    } break;
    // <string.h>
    case LibcStrlen: {
      ret_ty = size_ty;              // ret
      params.push_back(char_ptr_ty); // s
    } break;
    case LibcStrnlen: {
      ret_ty = size_ty;              // ret
      params.push_back(char_ptr_ty); // s
      params.push_back(size_ty);     // maxlen
    } break;
    case LibcStrcpy: {
      ret_ty = char_ptr_ty;          // ret
      params.push_back(char_ptr_ty); // dest
      params.push_back(char_ptr_ty); // src
    } break;
    case LibcStrncpy: {
      ret_ty = char_ptr_ty;          // ret
      params.push_back(char_ptr_ty); // dest
      params.push_back(char_ptr_ty); // src
      params.push_back(size_ty);     // maxlen
    } break;
    case LibcStrcat: {
      ret_ty = char_ptr_ty;          // ret
      params.push_back(char_ptr_ty); // s1
      params.push_back(char_ptr_ty); // s2
    } break;
    case LibcStrncat: {
      ret_ty = char_ptr_ty;          // ret
      params.push_back(char_ptr_ty); // s1
      params.push_back(char_ptr_ty); // s2
      params.push_back(size_ty);     // maxlen
    } break;
    case LibcStrcmp: {
      ret_ty = si32_ty;              // ret
      params.push_back(char_ptr_ty); // s1
      params.push_back(char_ptr_ty); // s2
    } break;
    case LibcStrncmp: {
      ret_ty = si32_ty;              // ret
      params.push_back(char_ptr_ty); // s1
      params.push_back(char_ptr_ty); // s2
      params.push_back(size_ty);     // n
    } break;
    case LibcStrstr: {
      ret_ty = char_ptr_ty;          // ret
      params.push_back(char_ptr_ty); // haystack
      params.push_back(char_ptr_ty); // needle
    } break;
    case LibcStrchr: {
      ret_ty = char_ptr_ty;          // ret
      params.push_back(char_ptr_ty); // s
      params.push_back(si32_ty);     // c
    } break;
    case LibcStrdup: {
      ret_ty = char_ptr_ty;          // ret
      params.push_back(char_ptr_ty); // s
    } break;
    case LibcStrndup: {
      ret_ty = char_ptr_ty;          // ret
      params.push_back(char_ptr_ty); // s
      params.push_back(size_ty);     // n
    } break;
    case LibcStrcpyCheck: {
      ret_ty = char_ptr_ty;          // ret
      params.push_back(char_ptr_ty); // dest
      params.push_back(char_ptr_ty); // src
      params.push_back(size_ty);     // maxlen
    } break;
    case LibcMemoryCopyCheck: {
      ret_ty = void_ptr_ty;          // ret
      params.push_back(void_ptr_ty); // dest
      params.push_back(void_ptr_ty); // src
      params.push_back(size_ty);     // length
      params.push_back(size_ty);     // maxlen
    } break;
    case LibcMemoryMoveCheck: {
      ret_ty = void_ptr_ty;          // ret
      params.push_back(void_ptr_ty); // dest
      params.push_back(void_ptr_ty); // src
      params.push_back(size_ty);     // length
      params.push_back(size_ty);     // maxlen
    } break;
    case LibcMemorySetCheck: {
      ret_ty = void_ptr_ty;          // ret
      params.push_back(void_ptr_ty); // ptr
      params.push_back(si32_ty);     // value
      params.push_back(size_ty);     // length
      params.push_back(size_ty);     // maxlen
    } break;
    case LibcStrcatCheck: {
      ret_ty = char_ptr_ty;          // ret
      params.push_back(char_ptr_ty); // s1
      params.push_back(char_ptr_ty); // s2
      params.push_back(size_ty);     // maxlen
    } break;
    case LibcppNew: {
      ret_ty = void_ptr_ty;      // ret
      params.push_back(size_ty); // size
    } break;
    case LibcppNewArray: {
      ret_ty = void_ptr_ty;      // ret
      params.push_back(size_ty); // size
    } break;
    case LibcppDelete: {
      ret_ty = void_ty;              // ret
      params.push_back(void_ptr_ty); // ptr
    } break;
    case LibcppDeleteArray: {
      ret_ty = void_ty;              // ret
      params.push_back(void_ptr_ty); // ptr
    } break;
    case LibcppAllocateException: {
      ret_ty = void_ptr_ty;      // ret
      params.push_back(size_ty); // size
    } break;
    case LibcppFreeException: {
      ret_ty = void_ty;              // ret
      params.push_back(void_ptr_ty); // ptr
    } break;
    case LibcppThrow: {
      ret_ty = void_ty;              // ret
      params.push_back(void_ptr_ty); // ptr
      params.push_back(void_ptr_ty); // ptr
      params.push_back(void_ptr_ty); // ptr
    } break;
    case LibcppBeginCatch: {
      ret_ty = void_ptr_ty;          // ret
      params.push_back(void_ptr_ty); // exception ptr
    } break;
    case LibcppEndCatch: {
      ret_ty = void_ty; // ret
    } break;
    default: {
      ikos_unreachable("unreachable");
    }
  }

  return FunctionType::get(ctx, ret_ty, params, var_arg);
}

std::string Intrinsic::short_name(ID id) {
  return short_name(id, /* template_ty = */ nullptr);
}

/// \brief Return the short name for a template type parameter
static std::string template_type_name(Type* ty) {
  ikos_assert(ty != nullptr);

  struct TypeVisitor {
    using ResultType = std::string;

    std::string operator()(VoidType*) const { return "void"; }

    std::string operator()(IntegerType* t) const {
      if (t->is_unsigned()) {
        return "ui" + std::to_string(t->bit_width());
      } else {
        return "si" + std::to_string(t->bit_width());
      }
    }

    std::string operator()(FloatType* t) const {
      switch (t->float_semantic()) {
        case Half:
          return "half";
        case Float:
          return "float";
        case Double:
          return "double";
        case X86_FP80:
          return "x86_fp80";
        case FP128:
          return "fp128";
        case PPC_FP128:
          return "ppc_fp128";
        default:
          ikos_unreachable("unknown float semantic");
      }
    }

    std::string operator()(PointerType* t) const {
      return template_type_name(t->pointee()) + "*";
    }

    std::string operator()(StructType*) const {
      ikos_unreachable("unreachable");
    }

    std::string operator()(ArrayType*) const {
      ikos_unreachable("unreachable");
    }

    std::string operator()(VectorType*) const {
      ikos_unreachable("unreachable");
    }

    std::string operator()(OpaqueType*) const {
      ikos_unreachable("unreachable");
    }

    std::string operator()(FunctionType*) const {
      ikos_unreachable("unreachable");
    }
  };

  return apply_visitor(TypeVisitor{}, ty);
}

std::string Intrinsic::short_name(ID id, Type* template_ty) {
  ikos_assert(id != NotIntrinsic);

  switch (id) {
    case MemoryCopy:
      return "memcpy";
    case MemoryMove:
      return "memmove";
    case MemorySet:
      return "memset";
    case VarArgStart:
      return "va_start";
    case VarArgEnd:
      return "va_end";
    case VarArgGet:
      return "va_arg";
    case VarArgCopy:
      return "va_copy";
    case StackSave:
      return "stacksave";
    case StackRestore:
      return "stackrestore";
    case LifetimeStart:
      return "lifetime.start";
    case LifetimeEnd:
      return "lifetime.end";
    case EhTypeidFor:
      return "eh.typeid.for";
    case Trap:
      return "trap";
    // <ikos/analyzer/intrinsic.h>
    case IkosAssert:
      return "ikos.assert";
    case IkosAssume:
      return "ikos.assume";
    case IkosNonDet:
      return "ikos.nondet." + template_type_name(template_ty);
    case IkosCounterInit:
      return "ikos.counter.init";
    case IkosCounterIncr:
      return "ikos.counter.incr";
    case IkosCheckMemAccess:
      return "ikos.check_mem_access";
    case IkosCheckStringAccess:
      return "ikos.check_string_access";
    case IkosAssumeMemSize:
      return "ikos.assume_mem_size";
    case IkosForgetMemory:
      return "ikos.forget_memory";
    case IkosAbstractMemory:
      return "ikos.abstract_memory";
    case IkosWatchMemory:
      return "ikos.watch_memory";
    case IkosPartitioningVar:
      return "ikos.partitioning.var." + template_type_name(template_ty);
    case IkosPartitioningJoin:
      return "ikos.partitioning.join";
    case IkosPartitioningDisable:
      return "ikos.partitioning.disable";
    case IkosPrintInvariant:
      return "ikos.print_invariant";
    case IkosPrintValues:
      return "ikos.print_values";
    // <stdlib.h>
    case LibcMalloc:
      return "libc.malloc";
    case LibcCalloc:
      return "libc.calloc";
    case LibcValloc:
      return "libc.valloc";
    case LibcAlignedAlloc:
      return "libc.aligned_alloc";
    case LibcRealloc:
      return "libc.realloc";
    case LibcFree:
      return "libc.free";
    case LibcAbs:
      return "libc.abs";
    case LibcRand:
      return "libc.rand";
    case LibcSrand:
      return "libc.srand";
    case LibcExit:
      return "libc.exit";
    case LibcAbort:
      return "libc.abort";
    // <errno.h>
    case LibcErrnoLocation:
      return "libc.errno_location";
    // <fcntl.h>
    case LibcOpen:
      return "libc.open";
    // <unistd.h>
    case LibcClose:
      return "libc.close";
    case LibcRead:
      return "libc.read";
    case LibcWrite:
      return "libc.write";
    // <stdio.h>
    case LibcGets:
      return "libc.gets";
    case LibcFgets:
      return "libc.fgets";
    case LibcGetc:
      return "libc.getc";
    case LibcFgetc:
      return "libc.fgetc";
    case LibcGetchar:
      return "libc.getchar";
    case LibcPuts:
      return "libc.puts";
    case LibcFputs:
      return "libc.fputs";
    case LibcPutc:
      return "libc.putc";
    case LibcFputc:
      return "libc.fputc";
    case LibcPrintf:
      return "libc.printf";
    case LibcFprintf:
      return "libc.fprintf";
    case LibcSprintf:
      return "libc.sprintf";
    case LibcSnprintf:
      return "libc.snprintf";
    case LibcScanf:
      return "libc.scanf";
    case LibcFscanf:
      return "libc.fscanf";
    case LibcSscanf:
      return "libc.sscanf";
    case LibcFopen:
      return "libc.fopen";
    case LibcFclose:
      return "libc.fclose";
    case LibcFflush:
      return "libc.fflush";
    // <string.h>
    case LibcStrlen:
      return "libc.strlen";
    case LibcStrnlen:
      return "libc.strnlen";
    case LibcStrcpy:
      return "libc.strcpy";
    case LibcStrncpy:
      return "libc.strncpy";
    case LibcStrcat:
      return "libc.strcat";
    case LibcStrncat:
      return "libc.strncat";
    case LibcStrcmp:
      return "libc.strcmp";
    case LibcStrncmp:
      return "libc.strncmp";
    case LibcStrstr:
      return "libc.strstr";
    case LibcStrchr:
      return "libc.strchr";
    case LibcStrdup:
      return "libc.strdup";
    case LibcStrndup:
      return "libc.strndup";
    case LibcStrcpyCheck:
      return "libc.strcpy.check";
    case LibcMemoryCopyCheck:
      return "libc.memcpy.check";
    case LibcMemoryMoveCheck:
      return "libc.memmove.check";
    case LibcMemorySetCheck:
      return "libc.memset.check";
    case LibcStrcatCheck:
      return "libc.strcat.check";
    case LibcppNew:
      return "libcpp.new";
    case LibcppNewArray:
      return "libcpp.new[]";
    case LibcppDelete:
      return "libcpp.delete";
    case LibcppDeleteArray:
      return "libcpp.delete[]";
    case LibcppAllocateException:
      return "libcpp.allocate_exception";
    case LibcppFreeException:
      return "libcpp.free_exception";
    case LibcppThrow:
      return "libcpp.throw";
    case LibcppBeginCatch:
      return "libcpp.begincatch";
    case LibcppEndCatch:
      return "libcpp.endcatch";
    default:
      ikos_unreachable("unreachable");
  }
}

std::string Intrinsic::long_name(ID id) {
  return Prefix + short_name(id);
}

std::string Intrinsic::long_name(ID id, Type* template_ty) {
  return Prefix + short_name(id, template_ty);
}

} // end namespace ar
} // end namespace ikos
