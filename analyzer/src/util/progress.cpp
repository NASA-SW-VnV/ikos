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

#include <algorithm>

#include <llvm/Support/Process.h>

#include <ikos/analyzer/util/progress.hpp>

namespace ikos {
namespace analyzer {

// ProgressLogger

ProgressLogger::ProgressLogger(std::ostream& out) : Logger(out) {}

// InteractiveProgressLogger

InteractiveProgressLogger::InteractiveProgressLogger(std::ostream& out,
                                                     std::size_t num_tasks,
                                                     std::size_t out_columns)
    : ProgressLogger(out),
      _current_task(0),
      _num_tasks(num_tasks),
      _out_columns(std::max(out_columns, std::size_t{3})) {}

void InteractiveProgressLogger::start_task(StringRef status) {
  // Update the current task
  this->_current_task++;

  // Update the status
  this->_status.clear();
  this->_status += '[';
  this->_status += std::to_string(this->_current_task);
  this->_status += '/';
  this->_status += std::to_string(this->_num_tasks);
  this->_status += "] ";
  this->_status += status;

  // Truncate status
  if (this->_status.length() > this->_out_columns) {
    this->_status.resize(this->_out_columns - 2);
    this->_status += "..";
  }

  // Print the state
  this->erase_line();
  this->print_status();
  this->_out.flush();
}

void InteractiveProgressLogger::start_logger() {}

void InteractiveProgressLogger::end_logger() {
  this->erase_line();
  this->_out.flush();
}

void InteractiveProgressLogger::start_message() {
  this->erase_line();
}

void InteractiveProgressLogger::end_message() {
  this->print_status();
  this->_out.flush();
}

void InteractiveProgressLogger::erase_line() {
  this->_out << "\r\033[K";
}

void InteractiveProgressLogger::print_status() {
  this->_out << this->_status;
}

// LinearProgressLogger

LinearProgressLogger::LinearProgressLogger(std::ostream& out,
                                           std::size_t num_tasks)
    : ProgressLogger(out), _current_task(0), _num_tasks(num_tasks) {}

void LinearProgressLogger::start_task(StringRef status) {
  this->_current_task++;
  this->_out << "[" << this->_current_task << "/" << this->_num_tasks << "] "
             << status << "\n";
  this->_out.flush();
}

void LinearProgressLogger::start_logger() {}

void LinearProgressLogger::end_logger() {
  this->_out.flush();
}

void LinearProgressLogger::start_message() {}

void LinearProgressLogger::end_message() {
  this->_out.flush();
}

// NoProgressLogger

NoProgressLogger::NoProgressLogger(std::ostream& out) : ProgressLogger(out) {}

void NoProgressLogger::start_task(StringRef /*status*/) {}

void NoProgressLogger::start_logger() {}

void NoProgressLogger::end_logger() {
  this->_out.flush();
}

void NoProgressLogger::start_message() {}

void NoProgressLogger::end_message() {
  this->_out.flush();
}

// make_progress_logger

std::unique_ptr< ProgressLogger > make_progress_logger(ProgressOption opt,
                                                       LogLevel level,
                                                       std::size_t num_tasks) {
  if (!log::is_enabled_for(level)) {
    return std::make_unique< NoProgressLogger >(std::cout);
  }

  switch (opt) {
    case ProgressOption::Auto: {
      if (llvm::sys::Process::StandardOutIsDisplayed()) {
        return std::make_unique<
            InteractiveProgressLogger >(std::cout,
                                        num_tasks,
                                        llvm::sys::Process::
                                            StandardOutColumns());
      } else {
        return std::make_unique< NoProgressLogger >(std::cout);
      }
    }
    case ProgressOption::Interactive: {
      return std::make_unique<
          InteractiveProgressLogger >(std::cout,
                                      num_tasks,
                                      llvm::sys::Process::StandardOutColumns());
    }
    case ProgressOption::Linear: {
      return std::make_unique< LinearProgressLogger >(std::cout, num_tasks);
    }
    case ProgressOption::None: {
      return std::make_unique< NoProgressLogger >(std::cout);
    }
    default: {
      ikos_unreachable("unreachable");
    }
  }
}

} // end namespace analyzer
} // end namespace ikos
