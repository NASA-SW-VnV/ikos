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

#include <exception>
#include <memory>
#include <string>

namespace ikos {
namespace core {

/// \brief Base class for ikos exceptions
class Exception : public std::exception {
public:
  /// \brief Create a new exception
  ///
  /// Note that a call to what() will return an empty string.
  Exception() noexcept = default;

  /// \brief Copy constructor
  Exception(const Exception&) noexcept = default;

  /// \brief Move constructor
  Exception(Exception&&) noexcept = default;

  /// \brief Copy assignment operator
  Exception& operator=(const Exception&) noexcept = default;

  /// \brief Move assignment operator
  Exception& operator=(Exception&&) noexcept = default;

  /// \brief Get the explanatory string
  const char* what() const noexcept override { return ""; }

  /// \brief Destructor
  ~Exception() override = default;

}; // end class Exception

/// \brief Exception for logical errors
class LogicError : public Exception {
private:
  /// \brief Explanatory message
  ///
  /// See https://clang.llvm.org/extra/clang-tidy/checks/cert-err60-cpp.html
  std::shared_ptr< const std::string > _msg;

public:
  /// \brief Constructor
  ///
  /// \param msg Explanatory message
  explicit LogicError(const std::string& msg)
      : _msg(std::make_shared< const std::string >(msg)) {}

  /// \brief Constructor
  ///
  /// \param msg Explanatory message
  explicit LogicError(const char* msg)
      : _msg(std::make_shared< const std::string >(msg)) {}

  /// \brief No default constructor
  LogicError() = delete;

  /// \brief Copy constructor
  LogicError(const LogicError&) noexcept = default;

  /// \brief Move constructor
  LogicError(LogicError&&) noexcept = default;

  /// \brief Copy assignment operator
  LogicError& operator=(const LogicError&) noexcept = default;

  /// \brief Move assignment operator
  LogicError& operator=(LogicError&&) noexcept = default;

  /// \brief Get the explanatory string
  const char* what() const noexcept override { return this->_msg->c_str(); }

  /// \brief Destructor
  ~LogicError() override = default;

}; // end class LogicError

} // end namespace core
} // end namespace ikos
