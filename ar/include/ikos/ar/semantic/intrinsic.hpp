/*******************************************************************************
 *
 * \file
 * \brief Intrinsic definitions
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

#pragma once

#include <string>

namespace ikos {
namespace ar {

// forward declaration
class Bundle;
class Type;
class FunctionType;

/// \brief Helper for intrinsics
class Intrinsic {
public:
  /// \brief Unique integer for each intrinsic
  enum ID {
    NotIntrinsic = 0,
    MemoryCopy,
    MemoryMove,
    MemorySet,
    VarArgStart,
    VarArgEnd,
    VarArgGet,
    VarArgCopy,
    StackSave,
    StackRestore,
    LifetimeStart,
    LifetimeEnd,
    EhTypeidFor,
    Trap,
    _BeginIkosIntrinsic,
    // <ikos/analyzer/intrinsic.h>
    IkosAssert,
    IkosAssume,
    IkosNonDet,
    IkosCounterInit,
    IkosCounterIncr,
    IkosCheckMemAccess,
    IkosCheckStringAccess,
    IkosAssumeMemSize,
    IkosForgetMemory,
    IkosAbstractMemory,
    IkosWatchMemory,
    IkosPartitioningVar,
    IkosPartitioningJoin,
    IkosPartitioningDisable,
    IkosPrintInvariant,
    IkosPrintValues,
    _EndIkosIntrinsic,
    _BeginLibcIntrinsic,
    // <stdlib.h>
    LibcMalloc,
    LibcCalloc,
    LibcValloc,
    LibcAlignedAlloc,
    LibcRealloc,
    LibcFree,
    LibcAbs,
    LibcRand,
    LibcSrand,
    LibcExit,
    LibcAbort,
    // <errno.h>
    LibcErrnoLocation,
    // <fcntl.h>
    LibcOpen,
    // <unistd.h>
    LibcClose,
    LibcRead,
    LibcWrite,
    // <stdio.h>
    LibcGets,
    LibcFgets,
    LibcGetc,
    LibcFgetc,
    LibcGetchar,
    LibcPuts,
    LibcFputs,
    LibcPutc,
    LibcFputc,
    LibcPrintf,
    LibcFprintf,
    LibcSprintf,
    LibcSnprintf,
    LibcScanf,
    LibcFscanf,
    LibcSscanf,
    LibcFopen,
    LibcFclose,
    LibcFflush,
    // <string.h>
    LibcStrlen,
    LibcStrnlen,
    LibcStrcpy,
    LibcStrncpy,
    LibcStrcat,
    LibcStrncat,
    LibcStrcmp,
    LibcStrncmp,
    LibcStrstr,
    LibcStrchr,
    LibcStrdup,
    LibcStrndup,
    LibcStrcpyCheck,
    LibcMemoryCopyCheck,
    LibcMemoryMoveCheck,
    LibcMemorySetCheck,
    LibcStrcatCheck,
    _EndLibcIntrinsic,
    _BeginLibcppIntrinsic,
    LibcppNew,
    LibcppNewArray,
    LibcppDelete,
    LibcppDeleteArray,
    LibcppAllocateException,
    LibcppFreeException,
    LibcppThrow,
    LibcppBeginCatch,
    LibcppEndCatch,
    _EndLibcppIntrinsic,
  };

  /// \brief Prefix for names of intrinsic functions, ie. "ar."
  constexpr static const char* Prefix = "ar.";

  /// \brief Get the type of an intrinsic function
  static FunctionType* type(Bundle*, ID);

  /// \brief Get the type of an intrinsic function with a type parameter
  static FunctionType* type(Bundle*, ID, Type*);

  /// \brief Get the short name of an intrinsic function
  static std::string short_name(ID);

  /// \brief Get the short name of an intrinsic function with a type parameter
  static std::string short_name(ID, Type*);

  /// \brief Get the long name of the given intrinsic, ie. short name + prefix
  static std::string long_name(ID);

  /// \brief Get the long name of the given intrinsic with a type parameter
  static std::string long_name(ID, Type*);

}; // end class Intrinsic

} // end namespace ar
} // end namespace ikos
