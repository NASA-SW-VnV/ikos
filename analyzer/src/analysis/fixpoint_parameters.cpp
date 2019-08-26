/*******************************************************************************
 *
 * \file
 * \brief Fixpoint parameters implementation
 *
 * Author: Thomas Bailleux
 *
 * Contributor: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2018-2019 United States Government as represented by the
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

#include <ikos/analyzer/analysis/fixpoint_parameters.hpp>

namespace ikos {
namespace analyzer {

// WideningHints

boost::optional< const MachineInt& > WideningHints::get(
    ar::BasicBlock* head) const {
  auto it = this->_map.find(head);
  if (it != this->_map.end()) {
    return it->second;
  } else {
    return boost::none;
  }
}

void WideningHints::add(ar::BasicBlock* head, const MachineInt& hint) {
  this->_map.try_emplace(head, hint);
}

// CodeFixpointParameters

CodeFixpointParameters::CodeFixpointParameters(
    WideningStrategy widening_strategy_,
    NarrowingStrategy narrowing_strategy_,
    unsigned widening_delay_,
    unsigned widening_period_,
    boost::optional< unsigned > narrowing_iterations_)
    : widening_strategy(widening_strategy_),
      narrowing_strategy(narrowing_strategy_),
      widening_delay(widening_delay_),
      widening_period(std::max(widening_period_, 1U)),
      narrowing_iterations(narrowing_iterations_) {}

// FixpointParameters

FixpointParameters::FixpointParameters(const AnalysisOptions& opts)
    : _default_params(opts.widening_strategy,
                      opts.narrowing_strategy,
                      opts.widening_delay,
                      opts.widening_period,
                      opts.narrowing_iterations) {
  // Store the parameters for functions with a given widening delay
  for (const auto& p : opts.widening_delay_functions) {
    ar::Function* fun = p.first;

    if (!fun->is_definition()) {
      continue;
    }

    auto fun_fixpoint_params =
        std::make_unique< CodeFixpointParameters >(this->_default_params);
    fun_fixpoint_params->widening_delay = p.second;
    this->_map.try_emplace(fun, std::move(fun_fixpoint_params));
  }
}

FixpointParameters::~FixpointParameters() = default;

CodeFixpointParameters& FixpointParameters::get(ar::Function* fun) {
  ikos_assert(fun->is_definition());
  auto it = this->_map.find(fun);
  if (it != this->_map.end()) {
    return *it->second;
  } else {
    auto fun_fixpoint_params =
        std::make_unique< CodeFixpointParameters >(this->_default_params);
    auto res = this->_map.try_emplace(fun, std::move(fun_fixpoint_params));
    ikos_assert(res.second);
    return *res.first->second;
  }
}

const CodeFixpointParameters& FixpointParameters::get(ar::Function* fun) const {
  ikos_assert(fun->is_definition());
  auto it = this->_map.find(fun);
  ikos_assert(it != this->_map.end());
  return *it->second;
}

void FixpointParameters::dump(std::ostream& o) const {
  // Print default parameters
  o << "default widening strategy: "
    << widening_strategy_str(this->_default_params.widening_strategy) << "\n";
  o << "default narrowing strategy: "
    << narrowing_strategy_str(this->_default_params.narrowing_strategy) << "\n";
  o << "default widening delay: " << this->_default_params.widening_delay
    << "\n";
  o << "default widening period: " << this->_default_params.widening_period
    << "\n";
  o << "default narrowing iterations: ";
  if (this->_default_params.narrowing_iterations) {
    o << *this->_default_params.narrowing_iterations << "\n";
  } else {
    o << "none\n";
  }

  // Print function parameters
  for (const auto& p : this->_map) {
    ar::Function* fun = p.first;
    CodeFixpointParameters& params = *p.second;

    if (params.widening_strategy != this->_default_params.widening_strategy) {
      o << fun->name() << " widening strategy: "
        << widening_strategy_str(params.widening_strategy) << "\n";
    }
    if (params.narrowing_strategy != this->_default_params.narrowing_strategy) {
      o << fun->name() << " narrowing strategy: "
        << narrowing_strategy_str(params.narrowing_strategy) << "\n";
    }
    if (params.widening_delay != this->_default_params.widening_delay) {
      o << fun->name() << " widening delay: " << params.widening_delay << "\n";
    }
    if (params.widening_period != this->_default_params.widening_period) {
      o << fun->name() << " widening period: " << params.widening_period
        << "\n";
    }
    if (params.narrowing_iterations !=
        this->_default_params.narrowing_iterations) {
      o << fun->name() << " narrowing iterations: ";
      if (params.narrowing_iterations) {
        o << *params.narrowing_iterations << "\n";
      } else {
        o << "none\n";
      }
    }

    for (const auto& hint : params.widening_hints) {
      o << fun->name() << " hint for ";
      hint.first->dump(o);
      o << ": " << hint.second << "\n";
    }
  }
}

} // end namespace analyzer
} // end namespace ikos
