/*******************************************************************************
 *
 * \file
 * \brief Utility functions to demangle C++ symbols.
 *
 * Authors: Maxime Arthaud
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

#include <string>

#include <boost/version.hpp>

#if BOOST_VERSION <= 105500
#include <boost/units/detail/utility.hpp>
#else
#include <boost/core/demangle.hpp>
#endif

#include <ikos/analyzer/support/string_ref.hpp>

namespace ikos {
namespace analyzer {

/// \brief Return true if the given symbol name is mangled
inline bool is_mangled(const char* name) {
  // No need to check the size because name is null-terminated
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  return name[0] == '_' && name[1] >= 'A' && name[1] <= 'Z';
}

/// \brief Return true if the given symbol name is mangled
inline bool is_mangled(StringRef name) {
  // Check the size, name is not null-terminated
  return name.size() >= 2 && is_mangled(name.data());
}

/// \brief Return the demangled symbol name, or name if it is not mangled
inline std::string demangle(const char* name) {
  if (is_mangled(name)) {
#if BOOST_VERSION <= 105500
    return boost::units::detail::demangle(name);
#else
    return boost::core::demangle(name);
#endif
  } else {
    return name;
  }
}

/// \brief Return the demangled symbol name, or name if it is not mangled
inline std::string demangle(const std::string& name) {
  return demangle(name.c_str());
}

/// \brief Return the demangled symbol name, or name if it is not mangled
inline std::string demangle(StringRef name) {
  // boost::core::demangle requires a null-terminated string
  return demangle(name.to_string());
}

/// \brief Return the demangled symbol name, or name if it is not mangled
inline std::string demangle(llvm::StringRef name) {
  // boost::core::demangle requires a null-terminated string
  return demangle(name.str());
}

} // end namespace analyzer
} // end namespace ikos
