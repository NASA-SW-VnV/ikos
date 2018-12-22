/*******************************************************************************
 *
 * \file
 * \brief Logging utilities
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2017-2018 United States Government as represented by the
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

#include <iostream>

#include <ikos/core/support/compiler.hpp>

#include <ikos/analyzer/support/string_ref.hpp>
#include <ikos/analyzer/util/color.hpp>

namespace ikos {
namespace analyzer {

/// \brief Log level, from least to most verbose
enum class LogLevel {
  None = 60,
  Critical = 50,
  Error = 40,
  Warning = 30,
  Info = 20,
  Debug = 10,
  All = 0
};

namespace log {

/// \brief Global logging level
extern LogLevel Level;

/// \brief Logging output stream
inline std::ostream& out() {
  return std::cout;
}

/// \brief Return true if a message of severity `level` would be displayed
inline bool is_enabled_for(LogLevel user_level) {
  return Level <= user_level;
}

/// \brief Logging stream for critical messages
inline std::ostream& critical_out() {
  return out() << "[" << color::on_red() << "CRITICAL" << color::off() << "] ";
}

/// \brief Log a critical message
inline void critical(StringRef message) {
  if (is_enabled_for(LogLevel::Critical)) {
    critical_out() << message << std::endl;
  }
}

/// \brief Logging stream for error messages
inline std::ostream& error_out() {
  return out() << "[" << color::on_red() << "ERROR" << color::off() << "] ";
}

/// \brief Log an error message
inline void error(StringRef message) {
  if (is_enabled_for(LogLevel::Error)) {
    error_out() << message << std::endl;
  }
}

/// \brief Logging stream for warning messages
inline std::ostream& warning_out() {
  return out() << "[" << color::bold_yellow() << "!" << color::off() << "] ";
}

/// \brief Log a warning message
inline void warning(StringRef message) {
  if (is_enabled_for(LogLevel::Warning)) {
    warning_out() << message << std::endl;
  }
}

/// \brief Logging stream for informative messages
inline std::ostream& info_out() {
  return out() << "[" << color::bold_blue() << "*" << color::off() << "] ";
}

/// \brief Log an informative message
inline void info(StringRef message) {
  if (ikos_unlikely(is_enabled_for(LogLevel::Info))) {
    info_out() << message << std::endl;
  }
}

/// \brief Logging stream for debug messages
inline std::ostream& debug_out() {
  return out() << "[" << color::magenta() << "." << color::off() << "] ";
}

/// \brief Log a debug message
inline void debug(StringRef message) {
  if (ikos_unlikely(is_enabled_for(LogLevel::Debug))) {
    debug_out() << message << std::endl;
  }
}

} // end namespace log

} // end namespace analyzer
} // end namespace ikos
