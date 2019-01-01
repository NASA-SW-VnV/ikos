/*******************************************************************************
 *
 * \file
 * \brief FixpointProfileAnalysis implementation used by the analysis
 *
 * Author: Thomas Bailleux
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

#include <ikos/core/fixpoint/wto.hpp>

#include <ikos/ar/semantic/statement.hpp>

#include <ikos/analyzer/analysis/fixpoint_profile.hpp>
#include <ikos/analyzer/support/cast.hpp>

namespace ikos {
namespace analyzer {

namespace {

class FixpointProfileWtoVisitor
    : public core::WtoComponentVisitor< ar::Code* > {
private:
  using WtoVertexT = core::WtoVertex< ar::Code* >;
  using WtoCycleT = core::WtoCycle< ar::Code* >;

private:
  llvm::DenseMap< ar::BasicBlock*, std::unique_ptr< core::MachineInt > >*
      _collector;

public:
  /// \brief Default constructor
  explicit FixpointProfileWtoVisitor(
      llvm::DenseMap< ar::BasicBlock*, std::unique_ptr< core::MachineInt > >*
          collector) {
    this->_collector = collector;
  }

  /// \brief Default copy constructor
  FixpointProfileWtoVisitor(const FixpointProfileWtoVisitor&) = delete;

  /// \brief Default move constructor
  FixpointProfileWtoVisitor(FixpointProfileWtoVisitor&&) = delete;

  /// \brief Delete copy assignment operator
  FixpointProfileWtoVisitor& operator=(const FixpointProfileWtoVisitor&) =
      delete;

  /// \brief Delete move assignment operator
  FixpointProfileWtoVisitor& operator=(FixpointProfileWtoVisitor&&) = delete;

  /// \brief Destructor
  ~FixpointProfileWtoVisitor() override = default;

  void visit(const WtoVertexT&) override {}

  void visit(const WtoCycleT& cycle) override {
    auto head = cycle.head();

    if (head->num_successors() > 1) {
      auto successor = *(head->successor_begin());
      if (auto constant = this->extract_constant(successor)) {
        this->_collector->try_emplace(head,
                                      std::make_unique< ar::MachineInt >(
                                          *constant));
      }
    }

    for (auto it = cycle.begin(), et = cycle.end(); it != et; ++it) {
      it->accept(*this);
    }
  }

  boost::optional< ar::MachineInt > extract_constant(ar::BasicBlock* bb) const {
    if (bb->empty()) {
      return boost::none;
    }

    // we have a comparison, check if there is a constant
    if (auto cmp = dyn_cast< ar::Comparison >(bb->front())) {
      ar::IntegerConstant* constant = nullptr;
      bool cst_left;

      if (cmp->left()->is_integer_constant()) {
        constant = cast< ar::IntegerConstant >(cmp->left());
        cst_left = true;
      } else if (cmp->right()->is_integer_constant()) {
        constant = cast< ar::IntegerConstant >(cmp->right());
        cst_left = false;
      } else {
        return boost::none;
      }

      ar::MachineInt value = constant->value();
      ar::MachineInt one(1, value.bit_width(), value.sign());
      bool overflow = false;

      // check if the comparison is <= or >=
      if (cmp->predicate() == ar::Comparison::UIGE ||
          cmp->predicate() == ar::Comparison::SIGE) {
        if (cst_left) {
          // case `cst >= var` <=> `cst + 1 > var`
          value = add(value, one, overflow);
        } else {
          // case `var >= cst` <=> `var > cst - 1`
          value = sub(value, one, overflow);
        }
      } else if (cmp->predicate() == ar::Comparison::UILE ||
                 cmp->predicate() == ar::Comparison::SILE) {
        if (cst_left) {
          // case `cst <= var` <=> `cst - 1 < var`
          value = sub(value, one, overflow);
        } else {
          // case `var <= cst` <=> `var < cst + 1`
          value = add(value, one, overflow);
        }
      }
      if (overflow) {
        return boost::none;
      }
      return value;
    } else {
      return boost::none;
    }
  }

}; // end class FixpointProfileWtoVisitor

} // end anonymous namespace

void FixpointProfileAnalysis::run() {
  auto bundle = this->_ctx.bundle;
  for (auto it = bundle->function_begin(), et = bundle->function_end();
       it != et;
       it++) {
    if (auto profile = this->analyze_function(*it)) {
      this->_map.try_emplace(*it, std::move(profile));
    }
  }
}

void FixpointProfileAnalysis::dump(std::ostream& o) const {
  for (const auto& item : this->_map) {
    o << "function " << item.first->name() << ':' << std::endl;
    item.second->dump(o);
    o << std::endl;
  }
}

std::unique_ptr< FixpointProfile > FixpointProfileAnalysis::analyze_function(
    ar::Function* fun) {
  if (!fun->is_definition()) {
    return nullptr;
  }

  std::unique_ptr< FixpointProfile > profile(new FixpointProfile(fun));
  FixpointProfileWtoVisitor visitor(&profile->_widening_hints);
  core::Wto< ar::Code* > wto(fun->body());
  wto.accept(visitor);
  if (!profile->empty()) {
    return profile;
  } else {
    return nullptr;
  }
}

boost::optional< const FixpointProfile& > FixpointProfileAnalysis::profile(
    ar::Function* fun) const {
  auto it = this->_map.find(fun);
  if (it != this->_map.end()) {
    return *(it->second);
  } else {
    return boost::none;
  }
}

boost::optional< const core::MachineInt& > FixpointProfile::widening_hint(
    ar::BasicBlock* bb) const {
  auto it = this->_widening_hints.find(bb);
  if (it == this->_widening_hints.end()) {
    return boost::none;
  } else {
    return *it->second;
  }
}

bool FixpointProfile::empty() const {
  return this->_widening_hints.empty();
}

void FixpointProfile::dump(std::ostream& o) const {
  for (const auto& item : this->_widening_hints) {
    o << " • ";
    item.first->dump(o);
    o << ": " << *item.second << std::endl;
  }
}

} // namespace analyzer
} // end namespace ikos
