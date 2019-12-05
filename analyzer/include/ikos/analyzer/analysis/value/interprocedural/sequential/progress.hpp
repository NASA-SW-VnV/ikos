/*******************************************************************************
 *
 * \file
 * \brief Progress logging utilities for the interprocedural value analysis
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

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

#include <boost/variant.hpp>

#include <ikos/ar/semantic/code.hpp>
#include <ikos/ar/semantic/statement.hpp>

#include <ikos/core/fixpoint/fwd_fixpoint_iterator.hpp>

#include <ikos/analyzer/analysis/call_context.hpp>
#include <ikos/analyzer/analysis/context.hpp>
#include <ikos/analyzer/analysis/option.hpp>
#include <ikos/analyzer/util/log.hpp>

namespace ikos {
namespace analyzer {
namespace value {
namespace interprocedural {
namespace sequential {

/// \brief Base class for interprocedural value analysis progress loggers
class ProgressLogger : public Logger {
protected:
  /// \brief Analysis context
  Context& _ctx;

public:
  /// \brief Constructor
  ProgressLogger(std::ostream& out, Context& ctx);

  /// \brief Start analyzing a cycle
  ///
  /// \param head Head of the cycle
  virtual void start_cycle(ar::BasicBlock* head) = 0;

  /// \brief Start a cycle iteration
  ///
  /// \param head Head of the cycle
  /// \param iteration Number of iteration
  /// \param kind Increasing or Decreasing
  virtual void start_cycle_iter(ar::BasicBlock* head,
                                unsigned iteration,
                                core::FixpointIterationKind kind) = 0;

  /// \brief End analyzing a cycle
  ///
  /// \param head Head of the cycle
  virtual void end_cycle(ar::BasicBlock* head) = 0;

  /// \brief Start analyzing a called function
  virtual void start_callee(CallContext* call_context, ar::Function* fun) = 0;

  /// \brief End analyzing a called function
  virtual void end_callee(CallContext* call_context, ar::Function* fun) = 0;

}; // end class ProgressLogger

/// \brief Interactive interprocedural value analysis progress logger
class InteractiveProgressLogger final : public ProgressLogger {
private:
  /// \brief Refresh rate of the stack frame
  static constexpr const std::chrono::seconds RefreshRate =
      std::chrono::seconds(2);

private:
  /// \brief Represents a call frame
  struct CallFrame {
    CallContext* call_context;
    ar::Function* function;

    bool operator==(const CallFrame& o) const {
      return this->call_context == o.call_context &&
             this->function == o.function;
    }
  };

  /// \brief Represents a cycle frame
  struct CycleFrame {
    ar::BasicBlock* head;
    unsigned iteration;
    core::FixpointIterationKind kind;

    bool operator==(const CycleFrame& o) const {
      return this->head == o.head && this->iteration == o.iteration &&
             this->kind == o.kind;
    }
  };

  /// \brief Represents a frame
  using Frame = boost::variant< CallFrame, CycleFrame >;

private:
  /// \brief Number of columns in the output stream
  std::size_t _out_columns;

  /// \brief Thread that displays the current progress on a regular basis
  std::thread _thread;

  /// \brief Whether the thread is running
  std::atomic_bool _running;

  /// \brief Condition variable to notify the thread to stop running
  std::condition_variable _event;

  /// \brief Mutex
  std::mutex _mutex;

  /// \brief Current stack frame
  std::vector< Frame > _current_stack_frame;

  /// \brief Displayed stack frame
  std::vector< Frame > _displayed_stack_frame;

public:
  /// \brief Constructor
  ///
  /// \param out Output stream
  /// \param ctx Analysis context
  /// \param out_columns Number of columns in the output stream
  InteractiveProgressLogger(std::ostream& out,
                            Context& ctx,
                            std::size_t out_columns);

  /// \brief No copy constructor
  InteractiveProgressLogger(const InteractiveProgressLogger&) = delete;

  /// \brief No move constructor
  InteractiveProgressLogger(InteractiveProgressLogger&&) = delete;

  /// \brief No copy assignment operator
  InteractiveProgressLogger& operator=(const InteractiveProgressLogger&) =
      delete;

  /// \brief No move assignment operator
  InteractiveProgressLogger& operator=(InteractiveProgressLogger&&) = delete;

  /// \brief Destructor
  ~InteractiveProgressLogger() override;

  /// \brief Start analyzing a cycle
  void start_cycle(ar::BasicBlock* head) override;

  /// \brief Start a cycle iteration
  void start_cycle_iter(ar::BasicBlock* head,
                        unsigned iteration,
                        core::FixpointIterationKind kind) override;

  /// \brief End analyzing a cycle
  void end_cycle(ar::BasicBlock* head) override;

  /// \brief Start analyzing a called function
  void start_callee(CallContext* call_context, ar::Function* fun) override;

  /// \brief End analyzing a called function
  void end_callee(CallContext* call_context, ar::Function* fun) override;

  /// \brief This is called once when the logger becomes active
  void start_logger() override;

  /// \brief This is called once when the logger becomes inactive
  void end_logger() override;

  /// \brief This is called once before writing a log message
  void start_message() override;

  /// \brief This is called once after writing a log message
  void end_message() override;

private:
  /// \brief Run in a thread
  void run();

  /// \brief Print the current stack frame
  ///
  /// Precondition: the current thread owns the mutex
  void print_stack_frame();

  /// \brief Print a frame
  ///
  /// Precondition: the current thread owns the mutex
  void print_frame(const Frame& frame);

  /// \brief Clear the displayed stack frame
  ///
  /// Precondition: the current thread owns the mutex
  void clear_displayed_stack_frame();

}; // end class InteractiveProgressLogger

/// \brief Linear interprocedural value analysis progress logger
class LinearProgressLogger final : public ProgressLogger {
public:
  /// \brief Constructor
  LinearProgressLogger(std::ostream& out, Context& ctx);

  /// \brief Start analyzing a cycle
  void start_cycle(ar::BasicBlock* head) override;

  /// \brief Start a cycle iteration
  void start_cycle_iter(ar::BasicBlock* head,
                        unsigned iteration,
                        core::FixpointIterationKind kind) override;

  /// \brief End analyzing a cycle
  void end_cycle(ar::BasicBlock* head) override;

  /// \brief Start analyzing a called function
  void start_callee(CallContext* call_context, ar::Function* fun) override;

  /// \brief End analyzing a called function
  void end_callee(CallContext* call_context, ar::Function* fun) override;

  /// \brief This is called once when the logger becomes active
  void start_logger() override;

  /// \brief This is called once when the logger becomes inactive
  void end_logger() override;

  /// \brief This is called once before writing a log message
  void start_message() override;

  /// \brief This is called once after writing a log message
  void end_message() override;

}; // end class LinearProgressLogger

/// \brief Progress logger that discards progress status
class NoProgressLogger final : public ProgressLogger {
public:
  /// \brief Constructor
  NoProgressLogger(std::ostream& out, Context& ctx);

  /// \brief Start analyzing a cycle
  void start_cycle(ar::BasicBlock* head) override;

  /// \brief Start a cycle iteration
  void start_cycle_iter(ar::BasicBlock* head,
                        unsigned iteration,
                        core::FixpointIterationKind kind) override;

  /// \brief End analyzing a cycle
  void end_cycle(ar::BasicBlock* head) override;

  /// \brief Start analyzing a called function
  void start_callee(CallContext* call_context, ar::Function* fun) override;

  /// \brief End analyzing a called function
  void end_callee(CallContext* call_context, ar::Function* fun) override;

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
/// \param ctx Analysis context
/// \param opt Progress option
/// \param level Log level
std::unique_ptr< ProgressLogger > make_progress_logger(Context& ctx,
                                                       ProgressOption opt,
                                                       LogLevel level);

} // end namespace sequential
} // end namespace interprocedural
} // end namespace value
} // end namespace analyzer
} // end namespace ikos
