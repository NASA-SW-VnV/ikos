/*******************************************************************************
 *
 * Common types and classes used by the analyses
 *
 * Author: Jorge A. Navas
 *
 * Contributors: Clement Decoodt
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
#ifndef ANALYZER_COMMON_HPP
#define ANALYZER_COMMON_HPP

#include <memory>
#include <sstream>
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>

#include <ikos/common/types.hpp>
#include <ikos/domains/discrete_domains.hpp>
#include <ikos/domains/intervals.hpp>
#include <ikos/number/z_number.hpp>

#include <analyzer/analysis/variable_name.hpp>

namespace analyzer {

class analyzer_error {
private:
  std::string _msg;

private:
  analyzer_error();

public:
  analyzer_error(const std::string& msg) : _msg(msg) {}

  virtual std::string message() { return this->_msg; }

  virtual void write(std::ostream& o) { o << this->message(); }

  virtual ~analyzer_error() {}

}; // end class analyzer_error

inline std::ostream& operator<<(std::ostream& o, analyzer_error& e) {
  e.write(o);
  return o;
}

//! Simple type for expressing analyses results
enum analysis_result { OK, ERROR, WARNING, UNREACHABLE };

std::string result_to_str(analysis_result r) {
  switch (r) {
    case OK:
      return "ok";
    case ERROR:
      return "error";
    case WARNING:
      return "warning";
    case UNREACHABLE:
      return "unreachable";
    default:
      throw analyzer_error("unreachable");
  }
  return "warning";
}

//! Display invariants/checks settings
enum class display_settings { ALL, FAIL, OFF };

//! Numerical type for indexed objects
typedef uint64_t index_t;

//! Integer type
typedef ikos::z_number number_t;

} // end namespace analyzer

namespace ikos {
namespace var_name_traits {

// forward declaration
template < typename VariableFactory, typename MemoryLocation, typename Number >
struct value_domain_cell_var;

// see value_domain.hpp
template <>
struct value_domain_cell_var< analyzer::VariableFactory,
                              analyzer::memloc_t,
                              ikos::z_number > {
  typename analyzer::VariableFactory::variable_name_t operator()(
      analyzer::VariableFactory& vfac,
      analyzer::memloc_t addr,
      ikos::z_number offset,
      ikos::z_number size) {
    return vfac.get_cell(addr, offset, size);
  }
};

// forward declaration
template < typename VariableFactory, typename MemoryLocation, typename Number >
struct summary_domain_cell_var;

// see summary_domain.hpp
template <>
struct summary_domain_cell_var< analyzer::VariableFactory,
                                analyzer::memloc_t,
                                ikos::z_number > {
  typename analyzer::VariableFactory::variable_name_t operator()(
      analyzer::VariableFactory& vfac,
      analyzer::memloc_t addr,
      ikos::z_number offset,
      bound< ikos::z_number > size,
      bool output) {
    return vfac.get_sum_cell(addr, offset, size, output);
  }
};

// forward declaration
template < typename VariableName >
struct summary_domain_tmp_var;

// see summary_domain.hpp
template <>
struct summary_domain_tmp_var< analyzer::varname_t > {
  analyzer::varname_t operator()(analyzer::varname_t v) {
    return v->var_factory().get_summary_tmp();
  }
};

// forward declaration
template < typename VariableName >
struct offset_var;

// see pointer_domain.hpp
template <>
struct offset_var< analyzer::varname_t > {
  analyzer::varname_t operator()(analyzer::varname_t p) {
    return p->var_factory().get_offset(p);
  }
};

} // end namespace var_name_traits
} // end namespace ikos

namespace analyzer {

inline std::string ntostr(ikos::z_number n) {
  std::ostringstream buf;
  buf << n;
  return buf.str();
}

typedef ikos::patricia_tree_set< varname_t > varname_set_t;

class PointerInfo {
  typedef varname_t VariableName;
  typedef memloc_t MemoryLocation;
  typedef std::unordered_map<
      VariableName,
      std::pair< ikos::discrete_domain< MemoryLocation >, ikos::z_interval > >
      ptr_map_t;
  std::shared_ptr< ptr_map_t > _ptr_map;

public:
  typedef ikos::discrete_domain< MemoryLocation > ptr_set_t;

  PointerInfo() : _ptr_map(new ptr_map_t()) {}

  // Note: the shallow copy is intended.
  PointerInfo(const PointerInfo& o) : _ptr_map(o._ptr_map) {}

  void add(VariableName v,
           const std::pair< ptr_set_t, ikos::z_interval >& info) {
    _ptr_map->insert(ptr_map_t::value_type(v, info));
  }

  void write(std::ostream& o) {
    o << "{";
    for (ptr_map_t::iterator I = _ptr_map->begin(), E = _ptr_map->end();
         I != E;) {
      ikos::index_traits< varname_t >::write(o, I->first);
      o << " --> ";
      std::pair< ptr_set_t, ikos::z_interval > p = I->second;
      o << "(";
      p.first.write(o);
      o << "," << p.second << ")";
      ++I;
      if (I != E)
        o << ",\n";
    }
    o << "}";
  }

  std::pair< ptr_set_t, ikos::z_interval > operator[](VariableName v) const {
    ptr_map_t::iterator it = _ptr_map->find(v);
    if (it == _ptr_map->end())
      return std::make_pair(ptr_set_t::top(), ikos::z_interval::top());
    else
      return it->second;
  }
};

} // namespace analyzer

namespace ikos {

std::ostream& operator<<(std::ostream& o, ikos::operation_t op) {
  switch (op) {
    case OP_ADDITION:
      o << " + ";
      break;
    case OP_SUBTRACTION:
      o << " - ";
      break;
    case OP_MULTIPLICATION:
      o << " * ";
      break;
    case OP_DIVISION:
      o << " / ";
      break;
    default:;
  }
  return o;
}

inline std::ostream& operator<<(std::ostream& o,
                                boost::optional< ikos::operation_t > op) {
  if (op) {
    o << *op;
  }
  return o;
}

} // end namespace ikos

#endif // ANALYZER_COMMON_HPP
