/*******************************************************************************
 *
 * \file
 * \brief Base class for formatter of the abstract representation
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

#include <ikos/ar/support/flags.hpp>

namespace ikos {
namespace ar {

/// \brief Base class for formatters
class Formatter {
public:
  /// \brief Formatting options
  enum FormatOption {
    NoOption = 0x0,

    /// \brief When formatting a statement, show the result type
    ShowResultType = 0x1,

    /// \brief When formatting a statement, show the operand types
    ShowOperandTypes = 0x2,

    /// \brief Order global variables and functions by name
    ///
    /// This is useful to have a deterministic output, for testing purposes.
    OrderGlobals = 0x4,

    /// \brief Default format
    DefaultFormat = ShowResultType,
  };

  /// \brief Formatting options
  using FormatOptions = Flags< FormatOption >;

protected:
  FormatOptions _opts;

public:
  /// \brief Public constructor
  explicit Formatter(FormatOptions opts = DefaultFormat) : _opts(opts) {}

  /// \brief Copy constructor
  Formatter(const Formatter&) noexcept = default;

  /// \brief Move constructor
  Formatter(Formatter&&) noexcept = default;

  /// \brief Copy assignment operator
  Formatter& operator=(const Formatter&) noexcept = default;

  /// \brief Move assignment operator
  Formatter& operator=(Formatter&&) noexcept = default;

  /// \brief Destructor
  ~Formatter() = default;

  /// \brief Show the result type of the statements
  bool show_result_type() const { return this->_opts.test(ShowResultType); }

  /// \brief Show the operand types of the statements
  bool show_operand_types() const { return this->_opts.test(ShowOperandTypes); }

  /// \brief Order global variables and functions
  bool order_globals() const { return this->_opts.test(OrderGlobals); }

}; // end class Formatter

IKOS_DECLARE_OPERATORS_FOR_FLAGS(Formatter::FormatOptions)

} // end namespace ar
} // end namespace ikos
