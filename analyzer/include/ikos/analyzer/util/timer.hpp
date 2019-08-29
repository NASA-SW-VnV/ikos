/*******************************************************************************
 *
 * \file
 * \brief Timer utilities
 *
 * Author: Maxime Arthaud
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

#include <chrono>
#include <exception>
#include <functional>
#include <string>

namespace ikos {
namespace analyzer {

/// \brief Timer class
class Timer {
public:
  using Clock = std::chrono::steady_clock;
  using TimePoint = std::chrono::time_point< Clock >;
  using Duration = std::chrono::duration< double >;

private:
  TimePoint _start;
  TimePoint _end;

public:
  /// \brief Constructor
  Timer() = default;

  /// \brief Copy constructor
  Timer(const Timer&) = default;

  /// \brief No move constructor
  Timer(Timer&&) = delete;

  /// \brief Copy assignment operator
  Timer& operator=(const Timer&) = default;

  /// \brief No move assignment operator
  Timer& operator=(Timer&&) = delete;

  /// \brief Destructor
  ~Timer() = default;

  /// \brief Start the timer
  void start() { this->_start = Clock::now(); }

  /// \brief Stop the timer
  void stop() { this->_end = Clock::now(); }

  /// \brief Return the elapsed time
  Duration elapsed() const {
    return std::chrono::duration_cast< Duration >(this->_end - this->_start);
  }

}; // end class Timer

/// \brief Timer on a scope
class ScopeTimer {
private:
  /// \brief Actual timer
  Timer _timer;

  /// \brief Function to run at the end of the scope
  std::function< void(Timer::Duration) > _callback;

public:
  /// \brief Constructor
  ///
  /// \param callback Called at the end of the scope, with the elapsed time
  template < typename Fn >
  explicit ScopeTimer(Fn&& callback) : _callback(std::forward< Fn >(callback)) {
    this->_timer.start();
  }

  /// \brief No copy constructor
  ScopeTimer(const ScopeTimer&) = delete;

  /// \brief No move constructor
  ScopeTimer(ScopeTimer&&) = delete;

  /// \brief No copy assignment operator
  ScopeTimer& operator=(const ScopeTimer&) = delete;

  /// \brief No move assignment operator
  ScopeTimer& operator=(ScopeTimer&&) = delete;

  /// \brief Destructor
  ~ScopeTimer() {
    this->_timer.stop();

    try {
      this->_callback(this->_timer.elapsed());
    } catch (...) {
      std::terminate();
    }
  }

}; // end class ScopeTimer

// forward declaration
class TimesTable;

/// \brief Timer that saves the elapsed time at the end of the scope in a
/// database
class ScopeTimerDatabase {
private:
  /// \brief Actual timer
  Timer _timer;

  /// \brief Times table
  TimesTable& _table;

  /// \brief Timer name
  std::string _name;

public:
  /// \brief Constructor
  ///
  /// \param table The times table
  /// \param name Timer name
  ScopeTimerDatabase(TimesTable& table, std::string name);

  /// \brief No copy constructor
  ScopeTimerDatabase(const ScopeTimerDatabase&) = delete;

  /// \brief No move constructor
  ScopeTimerDatabase(ScopeTimerDatabase&&) = delete;

  /// \brief No copy assignment operator
  ScopeTimerDatabase& operator=(const ScopeTimerDatabase&) = delete;

  /// \brief No move assignment operator
  ScopeTimerDatabase& operator=(ScopeTimerDatabase&&) = delete;

  /// \brief Destructor
  ~ScopeTimerDatabase();

}; // end class ScopeTimerDatabase

} // end namespace analyzer
} // end namespace ikos
