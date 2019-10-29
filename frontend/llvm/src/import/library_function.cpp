/*******************************************************************************
 *
 * \file
 * \brief Translation for known library functions (ie. malloc, free, etc.)
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

#include <ikos/frontend/llvm/import/exception.hpp>

#include "library_function.hpp"

namespace ikos {
namespace frontend {
namespace import {

ar::Function* LibraryFunctionImporter::function(llvm::StringRef name) {
  // ikos functions
  if (this->_enable_ikos) {
    // <ikos/analyzer/intrinsic.h>
    if (name == "__ikos_assert") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::IkosAssert);
    } else if (name == "__ikos_assume") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::IkosAssume);
    } else if (name == "__ikos_nondet_int") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::IkosNonDet,
                                               ar::IntegerType::si32(
                                                   this->_context));
    } else if (name == "__ikos_nondet_uint") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::IkosNonDet,
                                               ar::IntegerType::ui32(
                                                   this->_context));
    } else if (name == "__ikos_check_mem_access") {
      return this->_bundle->intrinsic_function(
          ar::Intrinsic::IkosCheckMemAccess);
    } else if (name == "__ikos_check_string_access") {
      return this->_bundle->intrinsic_function(
          ar::Intrinsic::IkosCheckStringAccess);
    } else if (name == "__ikos_assume_mem_size") {
      return this->_bundle->intrinsic_function(
          ar::Intrinsic::IkosAssumeMemSize);
    } else if (name == "__ikos_forget_mem") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::IkosForgetMemory);
    } else if (name == "__ikos_abstract_mem") {
      return this->_bundle->intrinsic_function(
          ar::Intrinsic::IkosAbstractMemory);
    } else if (name == "__ikos_watch_mem") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::IkosWatchMemory);
    } else if (name == "__ikos_partitioning_var_int") {
      return this->_bundle
          ->intrinsic_function(ar::Intrinsic::IkosPartitioningVar,
                               ar::IntegerType::si32(this->_context));
    } else if (name == "__ikos_partitioning_join") {
      return this->_bundle->intrinsic_function(
          ar::Intrinsic::IkosPartitioningJoin);
    } else if (name == "__ikos_partitioning_disable") {
      return this->_bundle->intrinsic_function(
          ar::Intrinsic::IkosPartitioningDisable);
    } else if (name == "__ikos_print_invariant") {
      return this->_bundle->intrinsic_function(
          ar::Intrinsic::IkosPrintInvariant);
    } else if (name == "__ikos_print_values") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::IkosPrintValues);
    }
  }

  // libc functions
  if (this->_enable_libc) {
    // <stdlib.h>
    if (name == "malloc") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcMalloc);
    } else if (name == "calloc") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcCalloc);
    } else if (name == "valloc") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcValloc);
    } else if (name == "aligned_alloc") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcAlignedAlloc);
    } else if (name == "realloc") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcRealloc);
    } else if (name == "free") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcFree);
    } else if (name == "abs") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcAbs);
    } else if (name == "rand") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcRand);
    } else if (name == "srand") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcSrand);
    } else if (name == "exit") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcExit);
    } else if (name == "abort") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcAbort);
    }
    // <errno.h>
    if (name == "__errno_location" || name == "__errno" || name == "_errno" ||
        name == "__error") {
      return this->_bundle->intrinsic_function(
          ar::Intrinsic::LibcErrnoLocation);
    }
    // <fcntl.h>
    if (name == "open") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcOpen);
    }
    // <unistd.h>
    if (name == "close") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcClose);
    } else if (name == "read" || name == "\x01_read") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcRead);
    } else if (name == "write") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcWrite);
    }
    // <stdio.h>
    if (name == "gets") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcGets);
    } else if (name == "fgets") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcFgets);
    } else if (name == "getc") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcGetc);
    } else if (name == "fgetc") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcFgetc);
    } else if (name == "getchar") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcGetchar);
    } else if (name == "puts") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcPuts);
    } else if (name == "fputs" || name == "\x01_fputs") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcFputs);
    } else if (name == "putc") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcPutc);
    } else if (name == "fputc") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcFputc);
    } else if (name == "printf") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcPrintf);
    } else if (name == "fprintf") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcFprintf);
    } else if (name == "sprintf") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcSprintf);
    } else if (name == "snprintf") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcSnprintf);
    } else if (name == "scanf") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcScanf);
    } else if (name == "fscanf" || name == "__isoc99_fscanf") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcFscanf);
    } else if (name == "sscanf") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcSscanf);
    } else if (name == "fopen" || name == "\x01_fopen") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcFopen);
    } else if (name == "fclose") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcFclose);
    } else if (name == "fflush") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcFflush);
    }
    // <string.h>
    if (name == "strlen") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcStrlen);
    } else if (name == "strnlen") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcStrnlen);
    } else if (name == "strcpy") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcStrcpy);
    } else if (name == "strncpy") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcStrncpy);
    } else if (name == "strcat") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcStrcat);
    } else if (name == "strncat") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcStrncat);
    } else if (name == "strcmp") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcStrcmp);
    } else if (name == "strncmp") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcStrncmp);
    } else if (name == "strstr") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcStrstr);
    } else if (name == "strchr") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcStrchr);
    } else if (name == "strdup") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcStrdup);
    } else if (name == "strndup") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcStrndup);
    } else if (name == "__strcpy_chk") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcStrcpyCheck);
    } else if (name == "__memcpy_chk") {
      return this->_bundle->intrinsic_function(
          ar::Intrinsic::LibcMemoryCopyCheck);
    } else if (name == "__memmove_chk") {
      return this->_bundle->intrinsic_function(
          ar::Intrinsic::LibcMemoryMoveCheck);
    } else if (name == "__memset_chk") {
      return this->_bundle->intrinsic_function(
          ar::Intrinsic::LibcMemorySetCheck);
    } else if (name == "__strcat_chk") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcStrcatCheck);
    }
  }

  // libc++ functions
  if (this->_enable_libcpp) {
    if (name == "_Znwm" || name == "_Znwy") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcppNew);
    } else if (name == "_Znam" || name == "_Znay") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcppNewArray);
    } else if (name == "_ZdlPv") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcppDelete);
    } else if (name == "_ZdaPv") {
      return this->_bundle->intrinsic_function(
          ar::Intrinsic::LibcppDeleteArray);
    } else if (name == "__cxa_allocate_exception") {
      return this->_bundle->intrinsic_function(
          ar::Intrinsic::LibcppAllocateException);
    } else if (name == "__cxa_free_exception") {
      return this->_bundle->intrinsic_function(
          ar::Intrinsic::LibcppFreeException);
    } else if (name == "__cxa_throw") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcppThrow);
    } else if (name == "__cxa_begin_catch") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcppBeginCatch);
    } else if (name == "__cxa_end_catch") {
      return this->_bundle->intrinsic_function(ar::Intrinsic::LibcppEndCatch);
    }
  }

  // not a known library function
  return nullptr;
}

} // end namespace import
} // end namespace frontend
} // end namespace ikos
