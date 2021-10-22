/*******************************************************************************
 *
 * \file
 * \brief Exception definitions
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

#include <ikos/core/exception.hpp>
#include <ikos/core/support/compiler.hpp>

namespace ikos {
namespace frontend {
namespace import {

/// \brief Exception for import errors
class ImportError : public core::Exception {
private:
  /// \brief Explanatory message
  ///
  /// See https://clang.llvm.org/extra/clang-tidy/checks/cert-err60-cpp.html
  std::shared_ptr< const std::string > _msg;

public:
  /// \brief Constructor
  ///
  /// \param msg Explanatory message
  explicit ImportError(const std::string& msg)
      : _msg(std::make_shared< const std::string >(msg)) {}

  /// \brief Constructor
  ///
  /// \param msg Explanatory message
  explicit ImportError(const char* msg)
      : _msg(std::make_shared< const std::string >(msg)) {}

  /// \brief No default constructor
  ImportError() = delete;

  /// \brief Copy constructor
  ImportError(const ImportError&) noexcept = default;

  /// \brief Move constructor
  ImportError(ImportError&&) noexcept = default;

  /// \brief Copy assignment operator
  ImportError& operator=(const ImportError&) noexcept = default;

  /// \brief Move assignment operator
  ImportError& operator=(ImportError&&) noexcept = default;

  /// \brief Get the explanatory string
  const char* what() const noexcept override;

  /// \brief Destructor
  ~ImportError() override;

}; // end class ImportError

/// \brief Exception for a mismatch between LLVM type and LLVM debug info
class TypeDebugInfoMismatch : public ImportError {
public:
  /// \brief Constructor
  ///
  /// \param msg Explanatory message
  explicit TypeDebugInfoMismatch(const std::string& msg) : ImportError(msg) {}

  /// \brief Constructor
  ///
  /// \param msg Explanatory message
  explicit TypeDebugInfoMismatch(const char* msg) : ImportError(msg) {}

  /// \brief No default constructor
  TypeDebugInfoMismatch() = delete;

  /// \brief Copy constructor
  TypeDebugInfoMismatch(const TypeDebugInfoMismatch&) noexcept = default;

  /// \brief Move constructor
  TypeDebugInfoMismatch(TypeDebugInfoMismatch&&) noexcept = default;

  /// \brief Copy assignment operator
  TypeDebugInfoMismatch& operator=(const TypeDebugInfoMismatch&) noexcept =
      default;

  /// \brief Move assignment operator
  TypeDebugInfoMismatch& operator=(TypeDebugInfoMismatch&&) noexcept = default;

  /// \brief Destructor
  ~TypeDebugInfoMismatch() override;

}; // end class TypeDebugInfoMismatch

/// \brief Check that a condition holds, otherwise throw ImportError
inline void check_import(bool condition, const char* msg) {
  if (ikos_unlikely(!condition)) {
    throw ImportError(msg);
  }
}

/// \brief Check that a condition holds, otherwise throw TypeDebugInfoMismatch
inline void check_match(bool condition, const char* msg) {
  if (ikos_unlikely(!condition)) {
    throw TypeDebugInfoMismatch(msg);
  }
}

} // end namespace import
} // end namespace frontend
} // end namespace ikos
