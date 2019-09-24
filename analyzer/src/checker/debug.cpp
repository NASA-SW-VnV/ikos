/*******************************************************************************
 *
 * \file
 * \brief Debug checker implementation
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

#include <boost/algorithm/string/predicate.hpp>

#include <ikos/analyzer/analysis/literal.hpp>
#include <ikos/analyzer/checker/debug.hpp>
#include <ikos/analyzer/database/table/operands.hpp>
#include <ikos/analyzer/support/cast.hpp>
#include <ikos/analyzer/util/demangle.hpp>
#include <ikos/analyzer/util/log.hpp>
#include <ikos/analyzer/util/source_location.hpp>

namespace ikos {
namespace analyzer {

DebugChecker::DebugChecker(Context& ctx) : Checker(ctx) {}

CheckerName DebugChecker::name() const {
  return CheckerName::Debug;
}

const char* DebugChecker::description() const {
  return "Debug checker";
}

void DebugChecker::check(ar::Statement* stmt,
                         const value::AbstractDomain& inv,
                         CallContext* /*call_context*/) {
  if (auto call = dyn_cast< ar::IntrinsicCall >(stmt)) {
    ar::Function* fun = call->called_function();

    switch (fun->intrinsic_id()) {
      case ar::Intrinsic::IkosPrintInvariant: {
        this->exec_print_invariant(call, inv);
      } break;
      case ar::Intrinsic::IkosPrintValues: {
        this->exec_print_values(call, inv);
      } break;
      default: {
        break;
      }
    }
  }
}

void DebugChecker::exec_print_invariant(ar::IntrinsicCall* call,
                                        const value::AbstractDomain& inv) {
  LogMessage msg = log::msg();
  this->display_stmt_location(msg, call);
  msg << "__ikos_print_invariant():\n";
  inv.dump(msg.stream());
  msg << "\n";
}

/// \brief Print the interval of an integer variable
static void print_interval(LogMessage& msg,
                           const std::string& repr,
                           const core::machine_int::Interval& i) {
  msg << "\t" << repr;
  if (i.is_bottom()) {
    msg << " is bottom\n";
  } else if (auto x = i.singleton()) {
    msg << " is " << *x << "\n";
  } else {
    msg << " is in [" << i.lb() << ", " << i.ub() << "]\n";
  }
}

/// \brief Remove the '&' at the beginning of a string
static std::string deref(std::string s) {
  if (!s.empty() && s[0] == '&') {
    return s.substr(1);
  } else {
    return s;
  }
}

/// \brief Fix the string textual representations
///
/// This transforms &"xxx"[0] into "xxx"
static std::string fix_string_repr(std::string s) {
  if (boost::starts_with(s, "&\"") && boost::ends_with(s, "\"[0]")) {
    return s.substr(1, s.size() - 4);
  } else {
    return s;
  }
}

/// \brief Textual representation of a memory location
static std::string memory_location_repr(MemoryLocation* mem) {
  if (auto local_mem = dyn_cast< LocalMemoryLocation >(mem)) {
    return deref(OperandsTable::repr(local_mem->local_var()));
  } else if (auto global_mem = dyn_cast< GlobalMemoryLocation >(mem)) {
    return deref(OperandsTable::repr(global_mem->global_var()));
  } else if (auto fun_mem = dyn_cast< FunctionMemoryLocation >(mem)) {
    return demangle(fun_mem->function()->name());
  } else if (auto aggregate_mem = dyn_cast< AggregateMemoryLocation >(mem)) {
    return OperandsTable::repr(aggregate_mem->internal_var());
  } else if (isa< AbsoluteZeroMemoryLocation >(mem)) {
    return "absolute zero";
  } else if (isa< ArgvMemoryLocation >(mem)) {
    return "argv";
  } else if (isa< LibcErrnoMemoryLocation >(mem)) {
    return "libc.errno";
  } else if (auto dyn_alloc_mem = dyn_cast< DynAllocMemoryLocation >(mem)) {
    ar::CallBase* call = dyn_alloc_mem->call();
    ar::Function* fun = call->code()->function();
    std::string r = "dyn_alloc:" + demangle(fun->name());
    SourceLocation loc = source_location(call);
    if (loc) {
      r += ':';
      r += std::to_string(loc.line());
      r += ':';
      r += std::to_string(loc.column());
    }
    return r;
  } else {
    ikos_unreachable("unexpected memory location");
  }
}

/// \brief Print the points-to set of a pointer variable
static void print_points_to(
    LogMessage& msg,
    const std::string& repr,
    const core::PointsToSet< MemoryLocation* >& points_to) {
  if (points_to.is_bottom()) {
    msg << "\t" << repr << " points-to set is bottom\n";
  } else if (points_to.is_top()) {
    msg << "\t" << repr << " points-to set is unknown\n";
  } else if (points_to.is_empty()) {
    msg << "\t" << repr << " points-to set is empty\n";
  } else if (auto mem = points_to.singleton()) {
    msg << "\t" << repr << " points to " << memory_location_repr(*mem) << "\n";
  } else {
    msg << "\t" << repr << " points to {";
    for (auto it = points_to.begin(), et = points_to.end(); it != et;) {
      msg << memory_location_repr(*it);
      ++it;
      if (it != et) {
        msg << ", ";
      }
    }
    msg << "}\n";
  }
}

/// \brief Print the nullity of a pointer variable
static void print_nullity(LogMessage& msg,
                          const std::string& repr,
                          const core::Nullity& nullity) {
  msg << "\t" << repr;
  if (nullity.is_bottom()) {
    msg << " is bottom\n";
  } else if (nullity.is_null()) {
    msg << " is null\n";
  } else if (nullity.is_non_null()) {
    msg << " is non-null\n";
  } else {
    msg << " may be null\n";
  }
}

static void print_uninitialized(LogMessage& msg,
                                const std::string& repr,
                                const core::Uninitialized& uninitialized) {
  msg << "\t" << repr;
  if (uninitialized.is_bottom()) {
    msg << " is bottom\n";
  } else if (uninitialized.is_uninitialized()) {
    msg << " is uninitialized\n";
  } else if (uninitialized.is_initialized()) {
    msg << " is initialized\n";
  } else {
    msg << " may be uninitialized\n";
  }
}

void DebugChecker::exec_print_values(ar::IntrinsicCall* call,
                                     const value::AbstractDomain& inv) {
  LogMessage msg = log::msg();
  this->display_stmt_location(msg, call);
  msg << "__ikos_print_values(";
  for (auto it = call->arg_begin(), et = call->arg_end(); it != et;) {
    msg << fix_string_repr(OperandsTable::repr(*it));
    ++it;
    if (it != et) {
      msg << ", ";
    }
  }
  msg << "):\n";

  if (inv.is_normal_flow_bottom()) {
    msg << "\tStatement is unreachable\n";
  } else if (call->num_arguments() <= 1) {
    msg << "\tMissing arguments\n";
  } else {
    for (auto it = call->arg_begin() + 1, et = call->arg_end(); it != et;
         ++it) {
      std::string repr = OperandsTable::repr(*it);
      const ScalarLit& v = this->_lit_factory.get_scalar(*it);

      if (v.is_machine_int_var()) {
        print_interval(msg, repr, inv.normal().int_to_interval(v.var()));
      } else if (v.is_floating_point_var()) {
        // ignored for now
      } else if (v.is_pointer_var()) {
        print_points_to(msg, repr, inv.normal().pointer_to_points_to(v.var()));

        print_interval(msg,
                       "offset of " + repr,
                       inv.normal().pointer_offset_to_interval(v.var()));

        print_nullity(msg, repr, inv.normal().nullity_to_nullity(v.var()));
      } else {
        msg << "\tArgument " << repr << " is not a variable\n";
        continue;
      }

      print_uninitialized(msg,
                          repr,
                          inv.normal().uninit_to_uninitialized(v.var()));
    }
  }
}

} // end namespace analyzer
} // end namespace ikos
