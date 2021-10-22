/*******************************************************************************
 *
 * \file
 * \brief Generate an AR bundle from a LLVM module
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

#include <llvm/IR/Module.h>

#include <ikos/ar/semantic/bundle.hpp>
#include <ikos/ar/semantic/context.hpp>
#include <ikos/ar/support/flags.hpp>

namespace ikos {
namespace frontend {
namespace import {

/// \brief Check if the given module has debug information
bool has_debug_info(llvm::Module&);

/// \brief Import from LLVM to AR
class Importer {
public:
  /// \brief Import options
  enum ImportOption : unsigned {
    NoOption = 0x0,

    /// \brief Use ikos intrinsics (such as ar.ikos.assert, etc.)
    EnableLibIkos = 0x1,

    /// \brief Use libc intrinsics (such as ar.libc.malloc, etc.)
    EnableLibc = 0x2,

    /// \brief Use libcpp intrinsics (such as ar.libcpp.new, etc.)
    EnableLibcpp = 0x4,

    /// \brief Allow incorrect debug information
    ///
    /// Allow mismatch of llvm types (llvm::Type) and debug info types
    /// (llvm::DIType).
    ///
    /// For instance, the associated llvm::DIType of a llvm.dbg.value might not
    /// match the llvm::Type of the associated llvm::Value. This happens because
    /// of aggressive optimizations or ABI requirements.
    ///
    /// Enable this option if `ikos-pp -opt=aggressive` was used.
    AllowMismatchDebugInfo = 0x8,

    /// \brief Default options
    DefaultOptions =
        EnableLibIkos | EnableLibc | EnableLibcpp | AllowMismatchDebugInfo,
  };

  /// \brief Import options
  using ImportOptions = ar::Flags< ImportOption >;

private:
  // AR Context
  ar::Context& _context;

public:
  /// \brief Public constructor
  explicit Importer(ar::Context& ctx) : _context(ctx) {}

  /// \brief Copy constructor
  Importer(const Importer&) noexcept = default;

  /// \brief Move constructor
  Importer(Importer&&) noexcept = default;

  /// \brief No copy assignment operator
  Importer& operator=(const Importer&) = delete;

  /// \brief No move assignment operator
  Importer& operator=(Importer&&) = delete;

  /// \brief Destructor
  ~Importer() = default;

  /// \brief Generate an AR bundle from a LLVM module
  ///
  /// \throws ImportError on errors
  ar::Bundle* import(llvm::Module&, ImportOptions opts = DefaultOptions);
};

IKOS_DECLARE_OPERATORS_FOR_FLAGS(Importer::ImportOptions)

} // end namespace import
} // end namespace frontend
} // end namespace ikos
