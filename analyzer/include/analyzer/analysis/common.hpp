/*******************************************************************************
 *
 * Common types and classes used by the analyses
 *
 * Author: Jorge A. Navas
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2016 United States Government as represented by the
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

#include <stdint.h>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>

#include <ikos/common/bignums.hpp>
#include <ikos/common/types.hpp>
#include <ikos/domains/discrete_domains.hpp>
#include <ikos/domains/intervals.hpp>

namespace analyzer {

//! Simple type for expressing analyses results
enum analysis_result { OK, ERR, WARNING, UNREACHABLE };

std::string tostr(analysis_result r) {
  switch (r) {
    case OK:
      return "ok";
    case ERR:
      return "error";
    case WARNING:
      return "warning";
    case UNREACHABLE:
      return "unreachable";
    default:
      assert(false && "unreachable");
  }
  return "warning";
}

//! Display invariants/checks settings
enum class display_settings { ALL, FAIL, OFF };

//! Numerical type for indexed objects
typedef uint64_t index_t;

//! Simple management for variable names
class VariableFactory : public boost::noncopyable {
public:
  class indexed_string {
    friend class VariableFactory;

    std::shared_ptr< std::string > _s;
    index_t _id;
    VariableFactory* _vfac;

    indexed_string(std::shared_ptr< std::string > s,
                   index_t id,
                   VariableFactory* vfac)
        : _s(s), _id(id), _vfac(vfac) {}

  public:
    indexed_string(const indexed_string& is)
        : _s(is._s), _id(is._id), _vfac(is._vfac) {}

    indexed_string& operator=(const indexed_string& is) {
      _s = is._s;
      _id = is._id;
      _vfac = is._vfac;
      return *this;
    }

    index_t index() const { return this->_id; }
    const std::string& name() const { return *this->_s; }

    bool operator<(indexed_string s) const { return (this->_id < s._id); }

    bool operator==(indexed_string s) const { return (this->_id == s._id); }

    bool operator!=(indexed_string s) const { return (this->_id != s._id); }

    VariableFactory& getVarFactory() { return *_vfac; }

    void write(std::ostream& o) { o << *_s; }

    friend std::ostream& operator<<(std::ostream& o, indexed_string s) {
      s.write(o);
      return o;
    }

  }; // class indexed_string

private:
  typedef std::unordered_map< std::string, indexed_string > map_t;

  index_t _next_id;
  map_t _map;

public:
  typedef indexed_string varname_t;

  VariableFactory() : _next_id(1) {}

  VariableFactory(index_t start_id) : _next_id(start_id) {}

  indexed_string operator[](const std::string& s) {
    map_t::iterator it = _map.find(s);
    if (it == _map.end()) {
      indexed_string is(std::shared_ptr< std::string >(new std::string(s)),
                        _next_id++,
                        this);
      _map.insert(std::pair< std::string, indexed_string >(s, is));
      return is;
    } else {
      return it->second;
    }
  }
}; // class VariableFactory

typedef VariableFactory::varname_t varname_t;

inline std::size_t hash_value(const varname_t& v) {
  return std::hash< index_t >()(v.index());
}

} // end namespace analyzer

namespace ikos {
namespace var_name_traits {

// forward declaration
template < typename VariableName, typename Number >
struct value_domain_cell_var;

// see value_domain.hpp
template < typename Number >
struct value_domain_cell_var< analyzer::varname_t, Number > {
  analyzer::varname_t operator()(analyzer::varname_t addr,
                                 Number offset,
                                 Number size) {
    std::ostringstream buf;
    buf << "C{" << addr.name() << "," << offset << "," << size << "}";
    std::string name = buf.str();
    return addr.getVarFactory()[name];
  }
};

// forward declaration
template < typename VariableName, typename Number >
struct summary_domain_cell_var;

// see summary_domain.hpp
template < typename Number >
struct summary_domain_cell_var< analyzer::varname_t, Number > {
  analyzer::varname_t operator()(analyzer::varname_t addr,
                                 Number offset,
                                 bound< Number > size,
                                 bool output) {
    std::ostringstream buf;
    buf << "C{" << addr.name() << "," << offset << "," << size << ","
        << (output ? "O" : "I") << "}";
    std::string name = buf.str();
    return addr.getVarFactory()[name];
  }
};

// forward declaration
template < typename VariableName >
struct summary_domain_tmp_var;

// see summary_domain.hpp
template <>
struct summary_domain_tmp_var< analyzer::varname_t > {
  analyzer::varname_t operator()(analyzer::varname_t v) {
    static analyzer::index_t i = 1;
    std::string name = "shadow.tmp." + std::to_string(i);
    i++;
    return v.getVarFactory()[name];
  }
};

} // end namespace var_name_traits
} // end namespace ikos

namespace std {

template <>
struct hash< analyzer::varname_t > {
  std::size_t operator()(const analyzer::varname_t& v) const {
    return std::hash< analyzer::index_t >()(v.index());
  }
};

} // end namespace std

namespace analyzer {

inline std::string ntostr(ikos::z_number n) {
  std::ostringstream buf;
  buf << n;
  return buf.str();
}

typedef ikos::discrete_domain< varname_t > varname_set_t;
typedef std::unordered_set< varname_t > unord_varname_set_t;
typedef boost::optional< unord_varname_set_t > opt_unord_varname_set_t;

class PointerInfo {
  typedef varname_t VariableName;
  typedef std::unordered_map< VariableName,
                              std::pair< ikos::discrete_domain< VariableName >,
                                         ikos::z_interval > > ptr_map_t;
  std::shared_ptr< ptr_map_t > _ptr_map;

public:
  typedef ikos::discrete_domain< VariableName > ptr_set_t;

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
      o << I->first << " --> ";
      std::pair< ptr_set_t, ikos::z_interval > p = I->second;
      o << "(";
      p.first.write(o);
      o << "," << p.second << ")";
      ++I;
      if (I != E)
        o << ",";
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
