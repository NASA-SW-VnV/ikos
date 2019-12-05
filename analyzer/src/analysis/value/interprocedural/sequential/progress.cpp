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

#include <llvm/Support/Process.h>

#include <ikos/analyzer/analysis/value/interprocedural/sequential/progress.hpp>
#include <ikos/analyzer/util/demangle.hpp>
#include <ikos/analyzer/util/source_location.hpp>

namespace ikos {
namespace analyzer {
namespace value {
namespace interprocedural {
namespace sequential {

// ProgressLogger

ProgressLogger::ProgressLogger(std::ostream& out, Context& ctx)
    : Logger(out), _ctx(ctx) {}

// InteractiveProgressLogger

constexpr const std::chrono::seconds InteractiveProgressLogger::RefreshRate;

InteractiveProgressLogger::InteractiveProgressLogger(std::ostream& out,
                                                     Context& ctx,
                                                     std::size_t out_columns)
    : ProgressLogger(out, ctx),
      _out_columns(std::max(out_columns, std::size_t{4})),
      _running(false) {}

InteractiveProgressLogger::~InteractiveProgressLogger() {
  if (this->_thread.joinable()) {
    // Ask the thread to stop running
    this->_running = false;

    // Notify the thread
    this->_event.notify_all();

    // Stop the progress thread nicely
    this->_thread.join();
  }
}

void InteractiveProgressLogger::start_logger() {
  // Sanity checks
  ikos_assert(!this->_thread.joinable());
  ikos_assert(!this->_running);
  ikos_assert(this->_current_stack_frame.empty());
  ikos_assert(this->_displayed_stack_frame.empty());

  // Notify the thread to keep running
  this->_running = true;

  // Start the progress thread
  this->_thread = std::thread(&InteractiveProgressLogger::run, this);
}

void InteractiveProgressLogger::end_logger() {
  // Sanity checks
  ikos_assert(this->_thread.joinable());
  ikos_assert(this->_running);

  // Ask the thread to stop running
  this->_running = false;

  // Notify the thread
  this->_event.notify_all();

  // Stop the progress thread nicely
  this->_thread.join();

  // Clear the stack frames (no need to lock the mutex here)
  this->_current_stack_frame.clear();
  this->_displayed_stack_frame.clear();
}

void InteractiveProgressLogger::start_cycle(ar::BasicBlock* head) {
  std::lock_guard< std::mutex > lock(this->_mutex);
  this->_current_stack_frame.emplace_back(
      CycleFrame{head, 0, core::FixpointIterationKind::Increasing});
}

void InteractiveProgressLogger::start_cycle_iter(
    ar::BasicBlock* head,
    unsigned iteration,
    core::FixpointIterationKind kind) {
  std::lock_guard< std::mutex > lock(this->_mutex);
  this->_current_stack_frame.pop_back();
  this->_current_stack_frame.emplace_back(CycleFrame{head, iteration, kind});
}

void InteractiveProgressLogger::end_cycle(ar::BasicBlock* /*head*/) {
  std::lock_guard< std::mutex > lock(this->_mutex);
  this->_current_stack_frame.pop_back();
}

void InteractiveProgressLogger::start_callee(CallContext* call_context,
                                             ar::Function* fun) {
  std::lock_guard< std::mutex > lock(this->_mutex);
  this->_current_stack_frame.emplace_back(CallFrame{call_context, fun});
}

void InteractiveProgressLogger::end_callee(CallContext* /*call_context*/,
                                           ar::Function* /*fun*/) {
  std::lock_guard< std::mutex > lock(this->_mutex);
  this->_current_stack_frame.pop_back();
}

void InteractiveProgressLogger::start_message() {
  // Lock the mutex, unlock in end_message()
  this->_mutex.lock();

  // Clear the displayed stack frame
  this->clear_displayed_stack_frame();
}

void InteractiveProgressLogger::end_message() {
  // Unlock the mutex
  this->_mutex.unlock();
}

void InteractiveProgressLogger::run() {
  // Required by std::condition_variable
  std::mutex event_mutex;
  std::unique_lock< std::mutex > event_lock(event_mutex);

  while (this->_running) {
    this->_event.wait_for(event_lock, RefreshRate);

    if (!this->_running) {
      break;
    }

    {
      std::lock_guard< std::mutex > lock(this->_mutex);
      this->print_stack_frame();
    }
  }

  {
    std::lock_guard< std::mutex > lock(this->_mutex);
    this->clear_displayed_stack_frame();
  }
}

void InteractiveProgressLogger::print_stack_frame() {
  // Precondition: the current thread owns the mutex

  // Clear the previous stack frame
  this->clear_displayed_stack_frame();

  // Print the new stack frame
  for (auto begin = this->_current_stack_frame.begin(),
            end = this->_current_stack_frame.end(),
            it = begin;
       it != end;
       ++it) {
    if (it != begin) {
      this->_out << "\n";
    }
    this->print_frame(*it);
  }

  // Flush
  this->_out.flush();

  // Update state
  this->_displayed_stack_frame = this->_current_stack_frame;
}

/// \brief Return the source location of the given statement
using analyzer::source_location;

/// \brief Return the source location of the given basic block
static SourceLocation source_location(ar::BasicBlock* entry) {
  llvm::SmallVector< ar::BasicBlock*, 4 > worklist;
  llvm::SmallPtrSet< ar::BasicBlock*, 4 > seen;

  worklist.push_back(entry);

  // Find the first statement with a source location
  while (!worklist.empty()) {
    ar::BasicBlock* bb = worklist.back();
    worklist.pop_back();

    if (!seen.insert(bb).second) {
      continue;
    }

    for (ar::Statement* stmt : *bb) {
      SourceLocation loc = source_location(stmt);
      if (loc) {
        return loc;
      }
    }

    for (auto it = bb->predecessor_begin(), et = bb->predecessor_end();
         it != et;
         ++it) {
      worklist.push_back(*it);
    }
  }

  return {};
}

/// \brief Return a call frame as a string
static std::string call_frame_string(CallContext* call_context,
                                     ar::Function* function,
                                     const boost::filesystem::path& wd) {
  auto loc = source_location(call_context->call());
  std::string r = source_location_string(loc, wd);
  r += ": Analyzing called function '";
  r += demangle(function->name());
  r += "'";
  return r;
}

/// \brief Return a cycle frame as a string
static std::string cycle_frame_string(ar::BasicBlock* head,
                                      unsigned iteration,
                                      core::FixpointIterationKind kind,
                                      const boost::filesystem::path& wd) {
  auto loc = source_location(head);
  std::string r = source_location_string(loc, wd);
  r += ": Analyzing loop [iteration ";
  r += std::to_string(iteration);
  switch (kind) {
    case core::FixpointIterationKind::Increasing: {
      r += "↑";
    } break;
    case core::FixpointIterationKind::Decreasing: {
      r += "↓";
    } break;
    default: {
      ikos_unreachable("unreachable");
    }
  }
  r += ']';
  return r;
}

void InteractiveProgressLogger::print_frame(const Frame& frame) {
  // Return a string for the given frame
  struct FrameVisitor : public boost::static_visitor< std::string > {
  private:
    const boost::filesystem::path& _wd;

  public:
    explicit FrameVisitor(const boost::filesystem::path& wd) : _wd(wd) {}

    std::string operator()(const CallFrame& call) const {
      return call_frame_string(call.call_context, call.function, this->_wd);
    }

    std::string operator()(const CycleFrame& cycle) const {
      return cycle_frame_string(cycle.head,
                                cycle.iteration,
                                cycle.kind,
                                this->_wd);
    }
  };

  // Generate a string for the given frame
  FrameVisitor vis(this->_ctx.wd);
  std::string line = boost::apply_visitor(vis, frame);

  // Truncate, if needed
  if (line.length() + 2 > this->_out_columns) {
    line.resize(this->_out_columns - 4);
    line += "..";
  }

  // Print the line
  this->_out << color::bold_blue() << "> " << color::off() << line;
}

void InteractiveProgressLogger::clear_displayed_stack_frame() {
  // Precondition: the current thread owns the mutex

  if (this->_displayed_stack_frame.empty()) {
    return;
  }

  // The cursor is at the end of the last line of the stack frame

  // Move cursor to the beginning of the line
  this->_out << "\r";

  if (this->_displayed_stack_frame.size() > 1) {
    // Move cursor up `size - 1` lines
    this->_out << "\033[" << (this->_displayed_stack_frame.size() - 1) << "A";
  }

  // Clear everything down
  this->_out << "\033[J";

  this->_out.flush();

  this->_displayed_stack_frame.clear();
}

// LinearProgressLogger

LinearProgressLogger::LinearProgressLogger(std::ostream& out, Context& ctx)
    : ProgressLogger(out, ctx) {}

void LinearProgressLogger::start_cycle(ar::BasicBlock* /*head*/) {}

void LinearProgressLogger::start_cycle_iter(ar::BasicBlock* head,
                                            unsigned iteration,
                                            core::FixpointIterationKind kind) {
  this->_out << color::bold_blue() << "> " << color::off()
             << cycle_frame_string(head, iteration, kind, this->_ctx.wd) << "\n"
             << std::flush;
}

void LinearProgressLogger::end_cycle(ar::BasicBlock* /*head*/) {}

void LinearProgressLogger::start_callee(CallContext* call_context,
                                        ar::Function* fun) {
  this->_out << color::bold_blue() << "> " << color::off()
             << call_frame_string(call_context, fun, this->_ctx.wd) << "\n"
             << std::flush;
}

void LinearProgressLogger::end_callee(CallContext* /*call_context*/,
                                      ar::Function* /*fun*/) {}

void LinearProgressLogger::start_logger() {}

void LinearProgressLogger::end_logger() {
  this->_out.flush();
}

void LinearProgressLogger::start_message() {}

void LinearProgressLogger::end_message() {
  this->_out.flush();
}

// NoProgressLogger

NoProgressLogger::NoProgressLogger(std::ostream& out, Context& ctx)
    : ProgressLogger(out, ctx) {}

void NoProgressLogger::start_cycle(ar::BasicBlock* /*head*/) {}

void NoProgressLogger::start_cycle_iter(ar::BasicBlock* /*head*/,
                                        unsigned /*iteration*/,
                                        core::FixpointIterationKind /*kind*/) {}

void NoProgressLogger::end_cycle(ar::BasicBlock* /*head*/) {}

void NoProgressLogger::start_callee(CallContext* /*call_context*/,
                                    ar::Function* /*fun*/) {}

void NoProgressLogger::end_callee(CallContext* /*call_context*/,
                                  ar::Function* /*fun*/) {}

void NoProgressLogger::start_logger() {}

void NoProgressLogger::end_logger() {
  this->_out.flush();
}

void NoProgressLogger::start_message() {}

void NoProgressLogger::end_message() {
  this->_out.flush();
}

// make_progress_logger

std::unique_ptr< ProgressLogger > make_progress_logger(Context& ctx,
                                                       ProgressOption opt,
                                                       LogLevel level) {
  if (!log::is_enabled_for(level)) {
    return std::make_unique< NoProgressLogger >(std::cout, ctx);
  }

  switch (opt) {
    case ProgressOption::Auto: {
      if (llvm::sys::Process::StandardOutIsDisplayed()) {
        return std::make_unique<
            InteractiveProgressLogger >(std::cout,
                                        ctx,
                                        llvm::sys::Process::
                                            StandardOutColumns());
      } else {
        return std::make_unique< NoProgressLogger >(std::cout, ctx);
      }
    }
    case ProgressOption::Interactive: {
      return std::make_unique<
          InteractiveProgressLogger >(std::cout,
                                      ctx,
                                      llvm::sys::Process::StandardOutColumns());
    }
    case ProgressOption::Linear: {
      return std::make_unique< LinearProgressLogger >(std::cout, ctx);
    }
    case ProgressOption::None: {
      return std::make_unique< NoProgressLogger >(std::cout, ctx);
    }
    default: {
      ikos_unreachable("unreachable");
    }
  }
}

} // end namespace sequential
} // end namespace interprocedural
} // end namespace value
} // end namespace analyzer
} // end namespace ikos
