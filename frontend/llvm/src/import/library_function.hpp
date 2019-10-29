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

#pragma once

#include <llvm/ADT/StringRef.h>

#include <ikos/ar/semantic/bundle.hpp>
#include <ikos/ar/semantic/function.hpp>

#include "import_context.hpp"

namespace ikos {
namespace frontend {
namespace import {

/// \brief Helper class to find known library functions
class LibraryFunctionImporter {
private:
  // AR context
  ar::Context& _context;

  // AR bundle
  ar::Bundle* _bundle;

  // Enable ikos functions
  bool _enable_ikos;

  // Enable libc functions
  bool _enable_libc;

  // Enable libc++ functions
  bool _enable_libcpp;

public:
  /// \brief Public constructor
  explicit LibraryFunctionImporter(ImportContext& ctx)
      : _context(ctx.ar_context),
        _bundle(ctx.bundle),
        _enable_ikos(ctx.opts.test(Importer::EnableLibIkos)),
        _enable_libc(ctx.opts.test(Importer::EnableLibc)),
        _enable_libcpp(ctx.opts.test(Importer::EnableLibcpp)) {}

  /// \brief Get the ar::Function of a known library function
  ///
  /// If the given name is the name of a well-known library function (such as
  /// malloc, free, etc.), returns the corresponding intrinsic ar::Function.
  /// Otherwise, return nullptr.
  ar::Function* function(llvm::StringRef name);

}; // end class LibraryFunctionImporter

} // end namespace import
} // end namespace frontend
} // end namespace ikos
