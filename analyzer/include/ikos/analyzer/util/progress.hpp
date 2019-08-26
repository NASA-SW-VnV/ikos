/*******************************************************************************
 *
 * \file
 * \brief Progress logging utilities
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2019 United States Government as represented by the
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

#include <memory>
#include <string>

#include <ikos/analyzer/analysis/option.hpp>
#include <ikos/analyzer/util/log.hpp>

namespace ikos {
namespace analyzer {

/// \brief Base class for loggers with progress report
class ProgressLogger : public Logger {
public:
  /// \brief Constructor
  explicit ProgressLogger(std::ostream& out);

  /// \brief Notify the beginning of a task with the given status message
  virtual void start_task(StringRef status) = 0;

}; // end class ProgressLogger

/// \brief Interactive progress logger
class InteractiveProgressLogger final : public ProgressLogger {
private:
  /// \brief Current task
  std::size_t _current_task;

  /// \brief Number of tasks
  std::size_t _num_tasks;

  /// \brief Current status
  std::string _status;

  /// \brief Number of columns in the output stream
  std::size_t _out_columns;

public:
  /// \brief Constructor
  ///
  /// \param out Output stream
  /// \param num_tasks Number of tasks
  /// \param out_columns Number of columns in the output stream
  InteractiveProgressLogger(std::ostream& out,
                            std::size_t num_tasks,
                            std::size_t out_columns);

  /// \brief Notify the beginning of a task with the given status message
  void start_task(StringRef status) override;

  /// \brief This is called once when the logger becomes active
  void start_logger() override;

  /// \brief This is called once when the logger becomes inactive
  void end_logger() override;

  /// \brief This is called once before writing a log message
  void start_message() override;

  /// \brief This is called once after writing a log message
  void end_message() override;

private:
  /// \brief Erase the current line
  void erase_line();

  /// \brief Print the current status
  void print_status();

}; // end class InteractiveProgressLogger

/// \brief Linear progress logger
class LinearProgressLogger final : public ProgressLogger {
private:
  /// \brief Current task
  std::size_t _current_task;

  /// \brief Number of tasks
  std::size_t _num_tasks;

public:
  /// \brief Constructor
  ///
  /// \param out Output stream
  /// \param num_tasks Number of tasks
  LinearProgressLogger(std::ostream& out, std::size_t num_tasks);

  /// \brief Notify the beginning of a task with the given status message
  void start_task(StringRef status) override;

  /// \brief This is called once when the logger becomes active
  void start_logger() override;

  /// \brief This is called once when the logger becomes inactive
  void end_logger() override;

  /// \brief This is called once before writing a log message
  void start_message() override;

  /// \brief This is called once after writing a log message
  void end_message() override;

}; // end class LinearProgressLogger

/// \brief Progress logger that discards status updates
class NoProgressLogger final : public ProgressLogger {
public:
  /// \brief Constructor
  explicit NoProgressLogger(std::ostream& out);

  /// \brief Notify the beginning of a task with the given status message
  void start_task(StringRef status) override;

  /// \brief This is called once when the logger becomes active
  void start_logger() override;

  /// \brief This is called once when the logger becomes inactive
  void end_logger() override;

  /// \brief This is called once before writing a log message
  void start_message() override;

  /// \brief This is called once after writing a log message
  void end_message() override;

}; // end class NoProgressLogger

/// \brief Create a progress logger
///
/// \param opt Progress option
/// \param level Log level
/// \param num_tasks Number of tasks
std::unique_ptr< ProgressLogger > make_progress_logger(ProgressOption opt,
                                                       LogLevel level,
                                                       std::size_t num_tasks);

/// \brief Create a progress logger
///
/// \param opt Progress option
/// \param level Log level
/// \param num_tasks Number of tasks
inline std::unique_ptr< ProgressLogger > make_progress_logger(
    ProgressOption opt, LogLevel level, std::ptrdiff_t num_tasks) {
  ikos_assert(num_tasks >= 0);
  return make_progress_logger(opt,
                              level,
                              static_cast< std::size_t >(num_tasks));
}

} // end namespace analyzer
} // end namespace ikos
