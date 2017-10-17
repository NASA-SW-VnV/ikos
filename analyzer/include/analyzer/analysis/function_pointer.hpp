/******************************************************************************
 *
 * Pointer analysis on function pointers only.
 * This pass is intended to be used as a pre-step for other analyses.
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

#ifndef ANALYZER_FUNCTION_POINTER_HPP
#define ANALYZER_FUNCTION_POINTER_HPP

#include <analyzer/analysis/common.hpp>
#include <analyzer/analysis/pointer.hpp>
#include <analyzer/ar-wrapper/cfg.hpp>
#include <analyzer/ikos-wrapper/iterators.hpp>

namespace analyzer {

class FunctionPointerPass : public ikos::writeable, public boost::noncopyable {
private:
  // We are using PointerPass::PTA to generate all pointer constraints.
  // We don't care about offsets, so we need to give to PTA a numerical
  // analysis that returns always top.

  // Fake abstract domain that returns always top
  template < typename Number, typename VariableName, typename MemoryLocation >
  class TopDomain {
  public:
    typedef Number number_t;
    typedef VariableName variable_name_t;
    typedef MemoryLocation memory_location_t;

  private:
    typedef ikos::interval< Number > interval_t;

  public:
    static TopDomain top() { return TopDomain(); }

  public:
    TopDomain() {}

    interval_t operator[](VariableName v) { return interval_t::top(); }
  };

  // Fake numerical analysis that returns always top
  template < typename AbsNumDomain >
  class NumericalAnalysis {
  public:
    NumericalAnalysis() {}

    AbsNumDomain operator[](Basic_Block_ref bb) { return AbsNumDomain::top(); }

    template < typename Statement >
    AbsNumDomain analyze_stmt(Statement stmt, AbsNumDomain pre) {
      return AbsNumDomain::top();
    }
  };

private:
  typedef varname_t VariableName;
  typedef memloc_t MemoryLocation;
  typedef ikos::z_number Number;
  typedef ikos::interval< Number > interval_t;
  typedef TopDomain< Number, VariableName, MemoryLocation > AbsNumDomain;
  typedef NumericalAnalysis< AbsNumDomain > numerical_analysis_t;
  typedef std::shared_ptr< numerical_analysis_t > numerical_analysis_ptr_t;
  typedef PointerPass::PTA< AbsNumDomain, numerical_analysis_t > pta_t;

private:
  CfgFactory& _cfg_fac;
  VariableFactory& _vfac;
  memory_factory& _mfac;
  LiteralFactory& _lfac;
  PointerInfo _pointer_info;

public:
  FunctionPointerPass(CfgFactory& cfg_fac,
                      VariableFactory& vfac,
                      memory_factory& mfac,
                      LiteralFactory& lfac)
      : _cfg_fac(cfg_fac), _vfac(vfac), _mfac(mfac), _lfac(lfac) {}

  inline CfgFactory& cfg_factory() { return _cfg_fac; }
  inline VariableFactory& var_factory() { return _vfac; }
  inline memory_factory& mem_factory() { return _mfac; }
  inline LiteralFactory& lit_factory() { return _lfac; }
  inline PointerInfo& pointer_info() { return _pointer_info; }

  void execute(Bundle_ref bundle) {
    std::cout << "** Generating pointer constraints ... " << std::endl;
    ikos::pta_system csts_system;
    pta_t::pt_var_map_t pt_var_map;
    pta_t::address_map_t address_map;
    numerical_analysis_ptr_t num_analysis(new numerical_analysis_t());

    FuncRange functions = ar::getFunctions(bundle);
    for (FuncRange::iterator it = functions.begin(); it != functions.end();
         ++it) {
      Function_ref fun = *it;
      std::shared_ptr< pta_t > visitor(new pta_t(bundle,
                                                 fun,
                                                 csts_system,
                                                 num_analysis,
                                                 _lfac,
                                                 _vfac,
                                                 _mfac,
                                                 pt_var_map,
                                                 address_map));
      ar::accept(fun, visitor);
    }

    std::cout << "** Solving pointer constraints ... " << std::endl;
    csts_system.solve();
    save_pointer_info(csts_system, pt_var_map, address_map);
  }

  void write(std::ostream& o) { _pointer_info.write(o); }

private:
  void save_pointer_info(
      const ikos::pta_system& csts_system, // solved constraints
      // mapping varname to pointer variables
      const pta_t::pt_var_map_t& pt_var_map,
      // mapping id's to varname
      const pta_t::address_map_t& address_map) {
    for (pta_t::pt_var_map_t::const_iterator it = pt_var_map.begin();
         it != pt_var_map.end();
         ++it) {
      const VariableName& var_name = it->first;
      const ikos::pointer_var& pt_var = it->second;

      ikos::address_set points_to_addrs = csts_system.get(pt_var).first;
      interval_t offset = csts_system.get(pt_var).second;
      if (offset.is_bottom()) {
        offset = interval_t::top();
      }

      if (points_to_addrs.empty()) {
        _pointer_info
            .add(var_name,
                 std::make_pair(ikos::discrete_domain< MemoryLocation >::top(),
                                offset));
      } else {
        ikos::discrete_domain< MemoryLocation > points_to_vars =
            ikos::discrete_domain< MemoryLocation >::bottom();

        for (ikos::address_set::const_iterator a_it = points_to_addrs.begin();
             a_it != points_to_addrs.end();
             ++a_it) {
          pta_t::address_map_t::const_iterator v_it = address_map.find(*a_it);
          assert(v_it != address_map.end());
          points_to_vars += v_it->second;
        }

        _pointer_info.add(var_name, std::make_pair(points_to_vars, offset));
      }
    }
  }
};

} // end namespace analyzer

#endif // ANALYZER_FUNCTION_POINTER_HPP
