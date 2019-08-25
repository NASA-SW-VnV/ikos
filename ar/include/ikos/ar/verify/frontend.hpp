/*******************************************************************************
 *
 * \file
 * \brief Check that statements have an attached front-end object
 *
 * See traceability.hpp for information on front-end object.
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

#include <iosfwd>

#include <ikos/ar/semantic/bundle.hpp>
#include <ikos/ar/semantic/code.hpp>
#include <ikos/ar/semantic/function.hpp>

namespace ikos {
namespace ar {

/// \brief Check that statements have an attached front-end object
class FrontendVerifier {
private:
  // Find all errors, do not stop at the first one
  bool _all;

public:
  /// \brief Public constructor
  ///
  /// \param all Find all errors, do not stop at the first one
  explicit FrontendVerifier(bool all = true) : _all(all) {}

  /// \brief Copy constructor
  FrontendVerifier(const FrontendVerifier&) noexcept = default;

  /// \brief Move constructor
  FrontendVerifier(FrontendVerifier&&) noexcept = default;

  /// \brief Copy assignment operator
  FrontendVerifier& operator=(const FrontendVerifier&) noexcept = default;

  /// \brief Move assignment operator
  FrontendVerifier& operator=(FrontendVerifier&&) noexcept = default;

  /// \brief Destructor
  ~FrontendVerifier() = default;

  /// \brief Check the given bundle
  ///
  /// \param err The output stream for errors
  bool verify(Bundle* bundle, std::ostream& err) const;

  /// \brief Check the given global variable
  ///
  /// \param err The output stream for errors
  bool verify(GlobalVariable* gv, std::ostream& err) const;

  /// \brief Check the given function
  ///
  /// \param err The output stream for errors
  bool verify(Function* f, std::ostream& err) const;

  /// \brief Check the given code
  ///
  /// \param err The output stream for errors
  bool verify(Code* code, std::ostream& err) const;

  /// \brief Check the given basic block
  ///
  /// \param err The output stream for errors
  bool verify(BasicBlock* bb, std::ostream& err) const;

}; // end class FrontendVerifier

} // end namespace ar
} // end namespace ikos
