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

#include <iostream>

#include <ikos/analyzer/util/log.hpp>

namespace ikos {
namespace analyzer {

// LoggerOutputStream

void LogMessage::start() {
  this->_logger->start_message();
}

void LogMessage::end() {
  this->_logger->end_message();
}

// TerminalLogger

TerminalLogger::TerminalLogger(std::ostream& out) noexcept : Logger(out) {}

void TerminalLogger::start_logger() {}

void TerminalLogger::end_logger() {
  this->_out.flush();
}

void TerminalLogger::start_message() {}

void TerminalLogger::end_message() {
  this->_out.flush();
}

// ScopeLogger

ScopeLogger::ScopeLogger(Logger& logger) : _previous_logger(log::get_logger()) {
  // Set the new logger
  log::set_logger(logger);
}

ScopeLogger::~ScopeLogger() {
  // Restore the previous logger
  log::set_logger(this->_previous_logger);
}

namespace log {

// Default global logging level
LogLevel Level = LogLevel::All;

// Default logger
static TerminalLogger DefaultLogger(std::cout);

/// \brief Global logger
static Logger* Log = &DefaultLogger;

void set_logger(Logger& logger) {
  Log->end_logger();
  Log = &logger;
  Log->start_logger();
}

Logger& get_logger() {
  return *Log;
}

} // end namespace log
} // end namespace analyzer
} // end namespace ikos
