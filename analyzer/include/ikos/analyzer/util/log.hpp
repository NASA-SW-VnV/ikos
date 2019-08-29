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

#pragma once

#include <iostream>

#include <ikos/core/support/compiler.hpp>

#include <ikos/analyzer/support/assert.hpp>
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

// forward declaration
class Logger;

/// \brief Represents a log message
class LogMessage {
private:
  friend class Logger;

private:
  /// \brief Logger
  Logger* _logger;

  /// \brief Output stream
  std::ostream* _out;

private:
  /// \brief Constructor
  LogMessage(Logger& logger, std::ostream& out) : _logger(&logger), _out(&out) {
    this->start();
  }

public:
  /// \brief No copy constructor
  LogMessage(const LogMessage&) = delete;

  /// \brief Move constructor
  LogMessage(LogMessage&& o) noexcept : _logger(o._logger), _out(o._out) {
    o._logger = nullptr;
    o._out = nullptr;
  }

  /// \brief No copy assignment operator
  LogMessage& operator=(const LogMessage&) = delete;

  /// \brief No move assignment operator
  LogMessage& operator=(LogMessage&&) = delete;

  /// \brief Return the output stream of the message
  std::ostream& stream() const {
    ikos_assert(this->_out != nullptr);
    return *this->_out;
  }

  /// \brief Destructor
  ~LogMessage() {
    if (this->_logger != nullptr) {
      this->end();
    }
  }

private:
  /// \brief Notify the logger of a new log message
  void start();

  /// \brief Notify the logger of the end of a log message
  void end();

}; // end class LoggerOutputStream

/// \brief Write a log message
template < typename T >
inline const LogMessage& operator<<(const LogMessage& msg, T&& v) {
  msg.stream() << std::forward< T >(v);
  return msg;
}

/// \brief Base class for loggers
class Logger {
private:
  friend class LogMessage;

protected:
  /// \brief Output stream
  std::ostream& _out;

public:
  /// \brief constructor
  explicit Logger(std::ostream& out) noexcept : _out(out) {}

  /// \brief No copy constructor
  Logger(const Logger&) = delete;

  /// \brief No move constructor
  Logger(Logger&&) = delete;

  /// \brief No copy assignment operator
  Logger& operator=(const Logger&) = delete;

  /// \brief No move assignment operator
  Logger& operator=(Logger&&) = delete;

  /// \brief Destructor
  virtual ~Logger() = default;

  /// \brief Return a new log message
  LogMessage create_message() { return {*this, this->_out}; }

  /// \brief This is called once when the logger becomes active
  virtual void start_logger() = 0;

  /// \brief This is called once when the logger becomes inactive
  virtual void end_logger() = 0;

  /// \brief This is called once before writing a log message
  virtual void start_message() = 0;

  /// \brief This is called once after writing a log message
  virtual void end_message() = 0;

}; // end class Logger

/// \brief Logger on the terminal
class TerminalLogger final : public Logger {
public:
  /// \brief Constructor
  explicit TerminalLogger(std::ostream& out) noexcept;

  /// \brief This is called once when the logger becomes active
  void start_logger() override;

  /// \brief This is called once when the logger becomes inactive
  void end_logger() override;

  /// \brief This is called once before writing a log message
  void start_message() override;

  /// \brief This is called once after writing a log message
  void end_message() override;

}; // end class TerminalLogger

/// \brief Setup a logger for a given scope
class ScopeLogger {
private:
  /// \brief Previous logger
  Logger& _previous_logger;

public:
  /// \brief Constructor
  explicit ScopeLogger(Logger& logger);

  /// \brief No copy constructor
  ScopeLogger(const ScopeLogger&) = delete;

  /// \brief No move constructor
  ScopeLogger(ScopeLogger&&) = delete;

  /// \brief No copy assignment operator
  ScopeLogger& operator=(const ScopeLogger&) = delete;

  /// \brief No move assignment operator
  ScopeLogger& operator=(ScopeLogger&&) = delete;

  /// \brief Destructor
  ~ScopeLogger();

}; // end class ScopeLogger

namespace log {

/// \brief Global logging level
extern LogLevel Level;

/// \brief Set the logger
void set_logger(Logger&);

/// \brief Get the logger
Logger& get_logger();

/// \brief Logging output stream
inline LogMessage msg() {
  return get_logger().create_message();
}

/// \brief Return true if a message of severity `level` would be displayed
inline bool is_enabled_for(LogLevel user_level) {
  return Level <= user_level;
}

/// \brief Log a critical message
inline void critical(StringRef message) {
  if (is_enabled_for(LogLevel::Critical)) {
    msg() << "[" << color::on_red() << "CRITICAL" << color::off() << "] "
          << message << "\n";
  }
}

/// \brief Log an error message
inline void error(StringRef message) {
  if (is_enabled_for(LogLevel::Error)) {
    msg() << "[" << color::on_red() << "ERROR" << color::off() << "] "
          << message << "\n";
  }
}

/// \brief Log a warning message
inline void warning(StringRef message) {
  if (is_enabled_for(LogLevel::Warning)) {
    msg() << "[" << color::bold_yellow() << "!" << color::off() << "] "
          << message << "\n";
  }
}

/// \brief Log an informative message
inline void info(StringRef message) {
  if (ikos_unlikely(is_enabled_for(LogLevel::Info))) {
    msg() << "[" << color::bold_blue() << "*" << color::off() << "] " << message
          << "\n";
  }
}

/// \brief Log a debug message
inline void debug(StringRef message) {
  if (ikos_unlikely(is_enabled_for(LogLevel::Debug))) {
    msg() << "[" << color::magenta() << "." << color::off() << "] " << message
          << "\n";
  }
}

} // end namespace log

} // end namespace analyzer
} // end namespace ikos
