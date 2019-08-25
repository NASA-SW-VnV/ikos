/*******************************************************************************
 *
 * \file
 * \brief Dot format for the abstract representation
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

#include <iosfwd>

#include <ikos/ar/format/formatter.hpp>
#include <ikos/ar/format/namer.hpp>
#include <ikos/ar/semantic/code.hpp>
#include <ikos/ar/semantic/function.hpp>
#include <ikos/ar/semantic/value.hpp>
#include <ikos/ar/support/string_ref.hpp>

namespace ikos {
namespace ar {

/// \brief Dot formatter
class DotFormatter : public Formatter {
public:
  /// \brief Public constructor
  explicit DotFormatter(FormatOptions opts = DefaultFormat) : Formatter(opts) {}

  /// \brief Copy constructor
  DotFormatter(const DotFormatter&) noexcept = default;

  /// \brief Move constructor
  DotFormatter(DotFormatter&&) noexcept = default;

  /// \brief Copy assignment operator
  DotFormatter& operator=(const DotFormatter&) noexcept = default;

  /// \brief Move assignment operator
  DotFormatter& operator=(DotFormatter&&) noexcept = default;

  /// \brief Destructor
  ~DotFormatter() = default;

  /// \brief Format a function into a .dot
  void format(std::ostream&, Function*) const;

  /// \brief Format a global variable into a .dot
  void format(std::ostream&, GlobalVariable*) const;

private:
  /// \brief Format a code into a .dot
  void format(std::ostream&, Code*) const;

  /// \brief Format a basic block into a .dot
  void format(std::ostream&, BasicBlock*, const Namer&) const;

  /// \brief Escape .dot forbidden characters (such as '{', '}')
  static std::string armor(StringRef);

}; // end class DotFormatter

} // end namespace ar
} // end namespace ikos
