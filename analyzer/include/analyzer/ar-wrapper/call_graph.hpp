/*******************************************************************************
 *
 * Call Graph wrapper
 * Build call graph and pointer information
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2017 United States Government as represented by the
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

#ifndef ANALYZER_CALL_GRAPH_HPP
#define ANALYZER_CALL_GRAPH_HPP

#include <boost/optional.hpp>

#include <arbos/semantics/ar.hpp>

#include <analyzer/analysis/common.hpp>
#include <analyzer/cast.hpp>

namespace arbos {

namespace ar {

namespace {

class PointerInfoWrapper : public FunPointersInfo {
public:
  typedef analyzer::PointerInfo PointerInfo;
  typedef analyzer::VariableFactory VariableFactory;
  typedef analyzer::varname_t VariableName;
  typedef PointerInfo::ptr_set_t ptr_set_t;

private:
  const Bundle_ref& _bundle;
  const PointerInfo& _ptr_info;
  VariableFactory& _vfac;

public:
  PointerInfoWrapper(const Bundle_ref& bundle,
                     const PointerInfo& ptr_info,
                     VariableFactory& vfac)
      : _bundle(bundle), _ptr_info(ptr_info), _vfac(vfac) {}

  std::vector< Function_ref > pointedBy(
      Internal_Variable_ref internal_var) const {
    VariableName v = _vfac.get_internal(internal_var);
    ptr_set_t ptr_set = _ptr_info[v].first;
    std::vector< Function_ref > result;

    if (ptr_set.is_top()) {
      std::cerr << "warning: unable to solve indirect call on variable " << v
                << std::endl;
      return std::vector< Function_ref >();
    }

    for (ptr_set_t::iterator it = ptr_set.begin(); it != ptr_set.end(); ++it) {
      if (!analyzer::isa< analyzer::function_memory_location >(*it))
        continue;
      auto memloc = analyzer::cast< analyzer::function_memory_location >(*it);
      boost::optional< Function_ref > fun =
          getFunction(_bundle, memloc->name());

      if (fun) { // not an external call
        result.push_back(*fun);
      }
    }

    return result;
  }
};

} // end anonymous namespace

// Build the call graph of a bundle.
// Points-to information are needed to solve indirect calls.
void buildCallGraph(const Bundle_ref& bundle,
                    const analyzer::PointerInfo& ptr_info,
                    analyzer::VariableFactory& vfac) {
  bundle->buildCallGraph(PointerInfoWrapper(bundle, ptr_info, vfac));
}

} // end namespace ar

} // end namespace arbos

#endif // ANALYZER_CALL_GRAPH_HPP
