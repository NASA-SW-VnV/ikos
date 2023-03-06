/*******************************************************************************
 *
 * \file
 * \brief Hardware addresses utility for the analyzer
 *
 * Author: Thomas Bailleux
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2019 United States Government as represented by the
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

#include <vector>

#include <llvm/Support/CommandLine.h>

#include <ikos/core/number/machine_int.hpp>
#include <ikos/core/value/machine_int/interval.hpp>

#include <ikos/ar/semantic/bundle.hpp>

#include <ikos/analyzer/exception.hpp>
#include <ikos/analyzer/support/string_ref.hpp>

namespace ikos {
namespace analyzer {

/// \brief Class that handles a set of readable/writable addresses
///
/// We can declare hardware addresses with an argument or by giving a file
class HardwareAddresses {
private:
  using Interval = core::machine_int::Interval;

private:
  /// \brief Range of hardware addresses
  std::vector< Interval > _address_ranges;

  const ar::DataLayout& _data_layout;

public:
  /// \brief Constructor
  HardwareAddresses(
      ar::Bundle* bundle,
      const llvm::cl::list< std::string >& hardware_addresses,
      const llvm::cl::opt< std::string >& hardware_addresses_file);

  /// \brief Add a range from a string
  void add_range(StringRef);

  /// \brief Add a range from a file
  void add_range_from_file(const std::string&);

  /// \brief Get all the ranges
  const std::vector< Interval >& ranges() const {
    return this->_address_ranges;
  }

  /// \brief Check if a range is included in one of the hardware addresses
  bool geq(const Interval& other) const;

  /// \brief Check if all hardware ranges don't meet with the given range
  bool is_meet_bottom(const Interval& other) const;

  /// \brief Dump the object, for debugging purpose
  void dump(std::ostream& o) const;

private:
  /// \brief Add a range
  void add_range(Interval);

}; // end class HardwareAddresses

/// \brief Exception for HardwareAddresses
class HardwareAddressesException : public analyzer::Exception {
public:
  /// \brief Enum of all kind of exception
  enum HardwareAddressesExceptionKind {
    InvalidFormatKind,
    InvalidRangeKind,
    CannotOpenFileKind,
  };

private:
  /// \brief Line number if it applies
  uint64_t _n_line;

  /// \brief Explanatory string
  std::shared_ptr< const std::string > _errstr;

  /// \brief Kind of exception
  HardwareAddressesExceptionKind _kind;

public:
  /// \brief Constructor
  ///
  /// \param pattern The pattern which is parsed
  /// \param kind Kind of exception
  explicit HardwareAddressesException(StringRef pattern,
                                      HardwareAddressesExceptionKind kind);

  /// \brief Constructor
  ///
  /// \param pattern The pattern which is parsed
  /// \param n_line Line number
  /// \param kind Kind of exception
  explicit HardwareAddressesException(StringRef pattern,
                                      HardwareAddressesExceptionKind kind,
                                      uint64_t n_line);

  /// \brief No default constructor
  HardwareAddressesException() = delete;

  /// \brief Copy constructor
  HardwareAddressesException(const HardwareAddressesException&) noexcept =
      default;

  /// \brief Move constructor
  HardwareAddressesException(HardwareAddressesException&&) noexcept = default;

  /// \brief Copy assignment operator
  HardwareAddressesException& operator=(
      const HardwareAddressesException&) noexcept = default;

  /// \brief Move assignment operator
  HardwareAddressesException& operator=(HardwareAddressesException&&) noexcept =
      default;

  /// \brief Get the explanatory string
  const char* what() const noexcept override;

  /// \brief Destructor
  ~HardwareAddressesException() override;

private:
  /// \brief Create the explanatory string
  void create_errstr(StringRef pattern);

}; // end class HardwareAddressesException

} // end namespace analyzer
} // end namespace ikos
