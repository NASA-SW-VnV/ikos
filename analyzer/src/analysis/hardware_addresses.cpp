/*******************************************************************************
 *
 * \file
 * \brief Implementation of hardware addresses utility for the analyzer
 *
 * Author: Thomas Bailleux
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

#include <fstream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>

#include <ikos/analyzer/analysis/hardware_addresses.hpp>

namespace ikos {
namespace analyzer {

namespace {

using Interval = core::machine_int::Interval;

/// \brief Get the range regex
const std::regex& range_regex() {
  static std::regex RangeRegex(
      R"(^[\s]*((?:0x)?[0-9a-f]+)[\s]*-[\s]*((?:0x)?[0-9a-f]+)[\s]*$)",
      std::regex_constants::icase);
  return RangeRegex;
}

Interval parse_range(StringRef buffer,
                     uint64_t n_line,
                     uint64_t ptr_bit_width) {
  uint64_t lbound;
  uint64_t ubound;
  std::cmatch rmatch;
  if (!std::regex_match(buffer.data(), rmatch, range_regex())) {
    throw HardwareAddressesException(buffer,
                                     HardwareAddressesException::
                                         HardwareAddressesExceptionKind::
                                             InvalidFormatKind,
                                     n_line);
  }
  std::string lbound_str = rmatch.str(0);
  std::string ubound_str = rmatch.str(2);
  try {
    lbound = std::stoull(lbound_str, nullptr, 0);
    ubound = std::stoull(ubound_str, nullptr, 0);
  } catch (const std::invalid_argument&) {
    throw HardwareAddressesException(buffer,
                                     HardwareAddressesException::
                                         HardwareAddressesExceptionKind::
                                             InvalidRangeKind,
                                     n_line);
  } catch (const std::out_of_range&) {
    throw HardwareAddressesException(buffer,
                                     HardwareAddressesException::
                                         HardwareAddressesExceptionKind::
                                             InvalidRangeKind,
                                     n_line);
  }

  if (lbound > ubound) {
    throw HardwareAddressesException(buffer,
                                     HardwareAddressesException::
                                         HardwareAddressesExceptionKind::
                                             InvalidRangeKind,
                                     n_line);
  }

  return Interval(core::MachineInt(lbound, ptr_bit_width, core::Unsigned),
                  core::MachineInt(ubound, ptr_bit_width, core::Unsigned));
}

} // end anonymous namespace

HardwareAddresses::HardwareAddresses(
    ar::Bundle* bundle,
    const llvm::cl::list< std::string >& hardware_addresses,
    const llvm::cl::opt< std::string >& hardware_addresses_file)
    : _data_layout(bundle->data_layout()) {
  if (!hardware_addresses.empty()) {
    for (const auto& range : hardware_addresses) {
      this->add_range(range);
    }
  }
  if (!hardware_addresses_file.empty()) {
    this->add_range_from_file(hardware_addresses_file.getValue());
  }
}

void HardwareAddresses::add_range(StringRef range_str) {
  this->add_range(
      parse_range(range_str, 0, this->_data_layout.pointers.bit_width));
}

void HardwareAddresses::add_range_from_file(const std::string& filepath) {
  std::ifstream stream(filepath);
  if (!stream.is_open()) {
    throw HardwareAddressesException(StringRef(),
                                     HardwareAddressesException::
                                         HardwareAddressesExceptionKind::
                                             CannotOpenFileKind);
  }
  // parse each line
  std::string line;
  uint64_t n_line = 1;
  for (; std::getline(stream, line); n_line++) {
    if (line.empty()) {
      continue;
    }
    std::cmatch rmatch;
    if (std::regex_match(line.c_str(), rmatch, range_regex())) {
      this->add_range(
          parse_range(line, n_line, this->_data_layout.pointers.bit_width));
    } else {
      throw HardwareAddressesException(line,
                                       HardwareAddressesException::
                                           HardwareAddressesExceptionKind::
                                               InvalidFormatKind,
                                       n_line);
    }
  }
}

void HardwareAddresses::add_range(Interval range) {
  this->_address_ranges.push_back(std::move(range));
}

bool HardwareAddresses::geq(const Interval& other) const {
  for (const auto& it : this->ranges()) {
    if (other.leq(it)) {
      return true;
    }
  }
  return false;
}

bool HardwareAddresses::is_meet_bottom(const Interval& other) const {
  for (const auto& it : this->ranges()) {
    if (!it.meet(other).is_bottom()) {
      return false;
    }
  }
  return true;
}

void HardwareAddresses::dump(std::ostream& o) const {
  o << "HardwareAddresses:\n";
  for (const auto& it : this->ranges()) {
    o << "  - " << it << "\n";
  }
}

HardwareAddressesException::HardwareAddressesException(
    StringRef pattern,
    HardwareAddressesException::HardwareAddressesExceptionKind kind)
    : _n_line(0), _kind(kind) {
  this->create_errstr(pattern);
}

HardwareAddressesException::HardwareAddressesException(
    StringRef pattern,
    HardwareAddressesException::HardwareAddressesExceptionKind kind,
    uint64_t n_line)
    : _n_line(n_line), _kind(kind) {
  this->create_errstr(pattern);
}

const char* HardwareAddressesException::what() const noexcept {
  return this->_errstr->c_str();
}

HardwareAddressesException::~HardwareAddressesException() = default;

void HardwareAddressesException::create_errstr(StringRef pattern) {
  std::stringstream ss;
  switch (this->_kind) {
    case InvalidFormatKind: {
      ss << "Invalid range format";
      if (!pattern.empty()) {
        ss << ": " << pattern.data();
      }
    } break;
    case InvalidRangeKind: {
      ss << "Invalid range";
      if (!pattern.empty()) {
        ss << ": " << pattern.data();
      }
    } break;
    case CannotOpenFileKind: {
      ss << "Cannot open hardware addresses file";
    } break;
  }

  if (this->_n_line != 0) {
    ss << ", at line " << this->_n_line;
  }

  this->_errstr = std::make_shared< const std::string >(ss.str());
}

} // end namespace analyzer
} // end namespace ikos
