/*******************************************************************************
 *
 * \file
 * \brief Source location for AR statements generated from LLVM
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2018-2019 United States Government as represented by the
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

#include <boost/filesystem.hpp>

#include <llvm/IR/DebugInfoMetadata.h>

#include <ikos/ar/semantic/statement.hpp>
#include <ikos/ar/support/assert.hpp>

namespace ikos {
namespace frontend {
namespace import {

/// \brief Return the absolute path of the given source file
boost::filesystem::path source_path(llvm::DIFile* file);

/// \brief Represents a source code location
class SourceLocation {
private:
  /// \brief Debug Information Location
  llvm::DILocation* _loc = nullptr;

public:
  /// \brief Create a null source location
  SourceLocation() = default;

  /// \brief Create a source location from an llvm::DILocation*
  explicit SourceLocation(llvm::DILocation* loc) : _loc(loc) {
    ikos_assert(loc != nullptr && loc->isResolved());
  }

  /// \brief Copy constructor
  SourceLocation(const SourceLocation&) noexcept = default;

  /// \brief Move constructor
  SourceLocation(SourceLocation&&) noexcept = default;

  /// \brief Copy assignment operator
  SourceLocation& operator=(const SourceLocation&) noexcept = default;

  /// \brief Move assignment operator
  SourceLocation& operator=(SourceLocation&&) noexcept = default;

  /// \brief Destructor
  ~SourceLocation() = default;

  /// \brief Return true if the source location is null
  bool is_null() const { return this->_loc == nullptr; }

  /// \brief Return true if the source location is not null
  explicit operator bool() const { return this->_loc != nullptr; }

  /// \brief Return the file
  llvm::DIFile* file() const {
    ikos_assert(this->_loc != nullptr);
    return this->_loc->getFile();
  }

  /// \brief Return the line
  unsigned line() const {
    ikos_assert(this->_loc != nullptr);
    return this->_loc->getLine();
  }

  /// \brief Return the column
  unsigned column() const {
    ikos_assert(this->_loc != nullptr);
    return this->_loc->getColumn();
  }

  /// \brief Return the absolute path to the filename
  boost::filesystem::path path() const { return source_path(this->file()); }

}; // end class SourceLocation

/// \brief Return the source location of the given statement
SourceLocation source_location(ar::Statement* stmt);

} // end namespace import
} // end namespace frontend
} // end namespace ikos
