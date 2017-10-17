/**************************************************************************/ /**
 *
 * \file
 * \brief Position-sensitive pointer analysis.
 *
 * This analysis operates on a simplified version of the constraint resolution
 * algorithm described in the following paper:
 *
 * Arnaud Venet. A Scalable Nonuniform Pointer Analysis for Embedded Programs.
 * In Proceedings of the International Static Analysis Symposium, SAS 04,
 * Verona, Italy. Lecture Notes in Computer Science, pages 149-164,
 * volume 3148, Springer 2004.
 *
 * Author: Arnaud J. Venet
 *
 * Contributors:
 *   * Jorge A. Navas
 *   * Maxime Arthaud
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

#ifndef IKOS_PTA_HPP
#define IKOS_PTA_HPP

#include <iostream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <ikos/common/types.hpp>
#include <ikos/number/z_number.hpp>
#include <ikos/value/interval.hpp>

namespace ikos {

typedef std::unordered_map< index64_t, index64_t > uid_map;
typedef std::unordered_set< index64_t > uid_set;

uid_map internal_to_external;
uid_set function_ids;
index64_t uid_counter = 0;

class pointer_var {
public:
  index64_t _uid;

public:
  pointer_var(index64_t uid) : _uid(uid) {}

  std::string str() const {
    std::stringstream buf;
    buf << "V_" << _uid;
    return buf.str();
  }

}; // end class pointer_var

bool operator<(pointer_var v1, pointer_var v2) {
  return v1._uid < v2._uid;
}

std::ostream& operator<<(std::ostream& o, pointer_var v) {
  uid_map::iterator it = internal_to_external.find(v._uid);
  if (it == internal_to_external.end()) {
    o << "v_" << v._uid;
  } else {
    o << "V{" << it->second << "}";
  }
  return o;
}

pointer_var mk_pointer_var() {
  return pointer_var(++uid_counter);
}

pointer_var mk_pointer_var(index64_t id) {
  index64_t uid = ++uid_counter;
  internal_to_external[uid] = id;
  return pointer_var(uid);
}

typedef enum {
  POINTER_REF,
  OBJECT_REF,
  FUNCTION_REF,
  PARAM_REF,
  RETURN_REF
} pta_ref_kind;

class pta_ref {
public:
  virtual pta_ref_kind kind() const = 0;

  virtual void print(std::ostream&) const = 0;

  virtual ~pta_ref() {}

}; // end class pta_ref

std::ostream& operator<<(std::ostream& o, const pta_ref& p) {
  p.print(o);
  return o;
}

class pointer_ref : public pta_ref {
public:
  pointer_var _pointer;
  z_interval _offset;

public:
  pointer_ref(pointer_var pointer, z_interval offset)
      : _pointer(pointer), _offset(offset) {}

  pta_ref_kind kind() const { return POINTER_REF; }

  void print(std::ostream& o) const {
    o << this->_pointer << " + " << this->_offset;
  }

  std::string str() const { return _pointer.str(); }

}; // end class pointer_ref

std::shared_ptr< pointer_ref > operator+(pointer_var v, z_interval o) {
  return std::shared_ptr< pointer_ref >(new pointer_ref(v, o));
}

class function_ref : public pta_ref {
public:
  index64_t _uid;

public:
  function_ref(index64_t uid) : _uid(uid) { function_ids.insert(uid); }

  pta_ref_kind kind() const { return FUNCTION_REF; }

  void print(std::ostream& o) const { o << "F{" << this->_uid << "}"; }

  std::string str() const {
    std::stringstream buf;
    buf << "F{" << this->_uid << "}";
    return buf.str();
  }

}; // end class function_ref

std::shared_ptr< function_ref > mk_function_ref(index64_t uid) {
  return std::shared_ptr< function_ref >(new function_ref(uid));
}

class object_ref : public pta_ref {
public:
  index64_t _address;
  z_interval _offset;

public:
  object_ref(index64_t address, z_interval offset)
      : _address(address), _offset(offset) {}

  pta_ref_kind kind() const { return OBJECT_REF; }

  void print(std::ostream& o) const {
    o << this->_address << " + " << this->_offset;
  }

  std::string str() const {
    std::stringstream buf;
    buf << "O{" << this->_address << "}";
    return buf.str();
  }

}; // end class object_ref

std::shared_ptr< object_ref > mk_object_ref(index64_t address,
                                            z_interval offset) {
  return std::shared_ptr< object_ref >(new object_ref(address, offset));
}

class param_ref : public pta_ref {
public:
  pointer_var _fptr;
  unsigned int _param;

public:
  param_ref(pointer_var fptr, unsigned int param)
      : _fptr(fptr), _param(param) {}

  pta_ref_kind kind() const { return PARAM_REF; }

  void print(std::ostream& o) const {
    o << "P_" << this->_param << "(" << this->_fptr << ")";
  }

  std::string str(index64_t fuid) const {
    std::ostringstream buf;
    buf << "P_" << this->_param << "(" << fuid << ")";
    return buf.str();
  }

}; // end class param_ref

std::shared_ptr< param_ref > mk_param_ref(pointer_var fptr,
                                          unsigned int param) {
  return std::shared_ptr< param_ref >(new param_ref(fptr, param));
}

class return_ref : public pta_ref {
public:
  pointer_var _fptr;

public:
  return_ref(pointer_var fptr) : _fptr(fptr) {}

  pta_ref_kind kind() const { return RETURN_REF; }

  void print(std::ostream& o) const { o << "R(" << this->_fptr << ")"; }

  std::string str(index64_t fuid) const {
    std::ostringstream buf;
    buf << "R(" << fuid << ")";
    return buf.str();
  }

}; // end class return_ref

std::shared_ptr< return_ref > mk_return_ref(pointer_var fptr) {
  return std::shared_ptr< return_ref >(new return_ref(fptr));
}

typedef enum { CST_ASSIGN, CST_STORE, CST_LOAD } pta_constraint_kind;

/// \brief Represents a pointer constraint
class pta_constraint {
public:
  virtual void print(std::ostream& o) const = 0;

  virtual pta_constraint_kind kind() const = 0;

  virtual ~pta_constraint() {}

}; // end class pta_constraint

std::ostream& operator<<(std::ostream& o, const pta_constraint& c) {
  c.print(o);
  return o;
}

class pta_assign : public pta_constraint {
public:
  pointer_var _lhs;
  std::shared_ptr< pta_ref > _rhs;

public:
  pta_assign(pointer_var lhs, std::shared_ptr< pta_ref > rhs)
      : _lhs(lhs), _rhs(rhs) {}

  void print(std::ostream& o) const {
    pta_ref& rhs = *(this->_rhs);
    o << this->_lhs << " => " << rhs;
  }

  pta_constraint_kind kind() const { return CST_ASSIGN; }

}; // end class pta_assign

std::shared_ptr< pta_constraint > operator==(
    pointer_var lhs, std::shared_ptr< pointer_ref > rhs) {
  return std::shared_ptr< pta_assign >(new pta_assign(lhs, rhs));
}

std::shared_ptr< pta_constraint > operator==(
    pointer_var lhs, std::shared_ptr< object_ref > rhs) {
  return std::shared_ptr< pta_assign >(new pta_assign(lhs, rhs));
}

std::shared_ptr< pta_constraint > operator==(
    pointer_var lhs, std::shared_ptr< function_ref > rhs) {
  return std::shared_ptr< pta_assign >(new pta_assign(lhs, rhs));
}

std::shared_ptr< pta_constraint > operator==(pointer_var lhs,
                                             std::shared_ptr< param_ref > rhs) {
  return std::shared_ptr< pta_assign >(new pta_assign(lhs, rhs));
}

std::shared_ptr< pta_constraint > operator==(
    pointer_var lhs, std::shared_ptr< return_ref > rhs) {
  return std::shared_ptr< pta_assign >(new pta_assign(lhs, rhs));
}

class pta_store : public pta_constraint {
public:
  std::shared_ptr< pta_ref > _lhs;
  pointer_var _rhs;

public:
  pta_store(std::shared_ptr< pta_ref > lhs, pointer_var rhs)
      : _lhs(lhs), _rhs(rhs) {}

  void print(std::ostream& o) const {
    pta_ref& lhs = *(this->_lhs);
    o << "*(" << lhs << ") => " << this->_rhs;
  }

  pta_constraint_kind kind() const { return CST_STORE; }

}; // end class pta_store

std::shared_ptr< pta_constraint > operator<<(std::shared_ptr< pta_ref > lhs,
                                             pointer_var rhs) {
  return std::shared_ptr< pta_store >(new pta_store(lhs, rhs));
}

class pta_load : public pta_constraint {
public:
  pointer_var _lhs;
  std::shared_ptr< pta_ref > _rhs;

public:
  pta_load(pointer_var lhs, std::shared_ptr< pta_ref > rhs)
      : _lhs(lhs), _rhs(rhs) {}

  void print(std::ostream& o) const {
    pta_ref& rhs = *(this->_rhs);
    o << this->_lhs << " => *(" << rhs << ")";
  }

  pta_constraint_kind kind() const { return CST_LOAD; }

}; // end class pta_load

std::shared_ptr< pta_constraint > operator*=(pointer_var lhs,
                                             std::shared_ptr< pta_ref > rhs) {
  return std::shared_ptr< pta_load >(new pta_load(lhs, rhs));
}

typedef std::unordered_set< index64_t > address_set;
typedef std::pair< address_set, z_interval > pta_info;

/// \brief System of pointer constraints
class pta_system {
  class binary_op {
  public:
    virtual z_interval apply(z_interval before,
                             z_interval after,
                             std::size_t iteration) const = 0;
    virtual bool convergence_achieved(z_interval before,
                                      z_interval after) const = 0;
    virtual ~binary_op() {}
  };

  class extrapolate : public binary_op {
  private:
    std::size_t _threshold;

  public:
    extrapolate(std::size_t threshold) : _threshold(threshold) {}

    bool convergence_achieved(z_interval before, z_interval after) const {
      return after <= before;
    }

    z_interval apply(z_interval before,
                     z_interval after,
                     std::size_t iteration) const {
      if (iteration < _threshold) {
        return before | after;
      } else {
        return before || after;
      }
    }
  };

  class refine : public binary_op {
    // This refinement is not a narrowing since it will not ensure
    // convergence. Thus, we need to make sure it is called a finite
    // number of times.

  public:
    refine() {}

    bool convergence_achieved(z_interval /*before*/,
                              z_interval /*after*/) const {
      return false;
    }

    z_interval apply(z_interval before,
                     z_interval after,
                     std::size_t /*iteration*/) const {
      if (after <= before) { // descending chain
        return before && after;
      } else { // no descending chain
        return before | after;
      }
    }
  };

private:
  typedef std::unordered_map< std::string, z_interval > offset_map;
  typedef std::unordered_map< std::string, address_set > address_map;

private:
  std::vector< std::shared_ptr< pta_constraint > > _csts;
  offset_map _offset_map;
  address_map _address_map;
  bool change_seen;
  std::size_t iteration;

private:
  address_set& get_address_set(const std::string& v) {
    return this->_address_map[v];
  }

  address_set get_address_set(const std::string& v) const {
    address_map::const_iterator it = this->_address_map.find(v);
    if (it == this->_address_map.end()) {
      return address_set();
    } else {
      return it->second;
    }
  }

  z_interval get_offset(const std::string& v) const {
    offset_map::const_iterator it = this->_offset_map.find(v);
    if (it == this->_offset_map.end()) {
      return z_interval::bottom();
    } else {
      return it->second;
    }
  }

  void add_offset(const std::string& v, z_interval o, const binary_op& op) {
    z_interval off = get_offset(v);
    if (!op.convergence_achieved(off, o)) {
      change_seen = true;
      z_interval new_offset = op.apply(off, o, iteration);
      offset_map::iterator it = this->_offset_map.find(v);
      if (it == this->_offset_map.end()) {
        this->_offset_map.insert(std::make_pair(v, new_offset));
      } else {
        it->second = new_offset;
      }
    }
  }

  void add_address(const std::string& v, index64_t a) {
    address_set& s = get_address_set(v);
    if (s.find(a) == s.end()) {
      change_seen = true;
      s.insert(a);
    }
  }

  void add_address_set(const std::string& v, address_set s) {
    for (address_set::iterator it = s.begin(); it != s.end(); ++it) {
      add_address(v, *it);
    }
  }

  pta_info process_ref(std::shared_ptr< pta_ref > ref) {
    switch (ref->kind()) {
      case POINTER_REF: {
        std::shared_ptr< pointer_ref > pt_ref =
            std::static_pointer_cast< pointer_ref >(ref);
        address_set& addrs = get_address_set(pt_ref->str());
        z_interval offset = get_offset(pt_ref->str());
        return std::make_pair(addrs, offset + pt_ref->_offset);
      }
      case OBJECT_REF: {
        std::shared_ptr< object_ref > obj_ref =
            std::static_pointer_cast< object_ref >(ref);
        address_set addrs;
        addrs.insert(obj_ref->_address);
        return std::make_pair(addrs, obj_ref->_offset);
      }
      case FUNCTION_REF: {
        std::shared_ptr< function_ref > fun_ref =
            std::static_pointer_cast< function_ref >(ref);
        address_set addrs;
        addrs.insert(fun_ref->_uid);
        return std::make_pair(addrs, z_interval::top());
      }
      case PARAM_REF: {
        std::shared_ptr< param_ref > param =
            std::static_pointer_cast< param_ref >(ref);
        address_set& fptrs = get_address_set(param->_fptr.str());
        address_set addrs;
        z_interval offset = z_interval::bottom();
        for (address_set::iterator it = fptrs.begin(); it != fptrs.end();
             ++it) {
          address_set& ptrs = get_address_set(param->str(*it));
          addrs.insert(ptrs.begin(), ptrs.end());
          offset = offset | get_offset(param->str(*it));
        }
        return std::make_pair(addrs, offset);
      }
      case RETURN_REF: {
        std::shared_ptr< return_ref > ret =
            std::static_pointer_cast< return_ref >(ref);
        address_set& fptrs = get_address_set(ret->_fptr.str());
        address_set addrs;
        z_interval offset = z_interval::bottom();
        for (address_set::iterator it = fptrs.begin(); it != fptrs.end();
             ++it) {
          address_set& ptrs = get_address_set(ret->str(*it));
          addrs.insert(ptrs.begin(), ptrs.end());
          offset = offset | get_offset(ret->str(*it));
        }
        return std::make_pair(addrs, offset);
      }
      default: { ikos_unreachable("invalid kind"); }
    }
  }

  void process_constraint(std::shared_ptr< pta_constraint > cst,
                          const binary_op& op) {
    switch (cst->kind()) {
      case CST_ASSIGN: {
        std::shared_ptr< pta_assign > assign =
            std::static_pointer_cast< pta_assign >(cst);
        pta_info rhs_info = process_ref(assign->_rhs);
        add_address_set(assign->_lhs.str(), rhs_info.first);
        add_offset(assign->_lhs.str(), rhs_info.second, op);
        break;
      }
      case CST_STORE: {
        std::shared_ptr< pta_store > store =
            std::static_pointer_cast< pta_store >(cst);
        address_set& rhs_addrs = get_address_set(store->_rhs.str());
        z_interval rhs_offset = get_offset(store->_rhs.str());
        std::shared_ptr< pta_ref > ref = store->_lhs;

        switch (ref->kind()) {
          case POINTER_REF: {
            std::shared_ptr< pointer_ref > pt_ref =
                std::static_pointer_cast< pointer_ref >(ref);
            address_set& addrs = get_address_set(pt_ref->str());
            for (address_set::iterator it = addrs.begin(); it != addrs.end();
                 ++it) {
              std::stringstream buf;
              buf << "O{" << *it << "}";
              add_address_set(buf.str(), rhs_addrs);
              add_offset(buf.str(), rhs_offset, op);
            }
            break;
          }
          case OBJECT_REF: {
            throw logic_error("pta system: cannot dereference a memory object");
            break;
          }
          case FUNCTION_REF: {
            throw logic_error(
                "pta system: cannot dereference a function object");
            break;
          }
          case PARAM_REF: {
            std::shared_ptr< param_ref > param =
                std::static_pointer_cast< param_ref >(ref);
            address_set& fptrs = get_address_set(param->_fptr.str());
            for (address_set::iterator it = fptrs.begin(); it != fptrs.end();
                 ++it) {
              add_address_set(param->str(*it), rhs_addrs);
              add_offset(param->str(*it), rhs_offset, op);
            }
            break;
          }
          case RETURN_REF: {
            std::shared_ptr< return_ref > ret =
                std::static_pointer_cast< return_ref >(ref);
            address_set& fptrs = get_address_set(ret->_fptr.str());
            for (address_set::iterator it = fptrs.begin(); it != fptrs.end();
                 ++it) {
              add_address_set(ret->str(*it), rhs_addrs);
              add_offset(ret->str(*it), rhs_offset, op);
            }
            break;
          }
          default: { ikos_unreachable("invalid kind"); }
        }
        break;
      }
      case CST_LOAD: {
        std::shared_ptr< pta_load > load =
            std::static_pointer_cast< pta_load >(cst);
        std::shared_ptr< pta_ref > ref = load->_rhs;

        switch (ref->kind()) {
          case POINTER_REF: {
            std::shared_ptr< pointer_ref > pt_ref =
                std::static_pointer_cast< pointer_ref >(ref);
            address_set& addrs = get_address_set(pt_ref->str());
            for (address_set::iterator it = addrs.begin(); it != addrs.end();
                 ++it) {
              std::stringstream buf;
              buf << "O{" << *it << "}";
              add_address_set(load->_lhs.str(), get_address_set(buf.str()));
              add_offset(load->_lhs.str(), get_offset(buf.str()), op);
            }
            break;
          }
          case OBJECT_REF: {
            throw logic_error("pta system: cannot dereference a memory object");
            break;
          }
          case FUNCTION_REF: {
            throw logic_error(
                "pta system: cannot dereference a function object");
            break;
          }
          case PARAM_REF: {
            std::shared_ptr< param_ref > param =
                std::static_pointer_cast< param_ref >(ref);
            address_set& fptrs = get_address_set(param->_fptr.str());
            for (address_set::iterator it = fptrs.begin(); it != fptrs.end();
                 ++it) {
              add_address_set(load->_lhs.str(),
                              get_address_set(param->str(*it)));
              add_offset(load->_lhs.str(), get_offset(param->str(*it)), op);
            }
            break;
          }
          case RETURN_REF: {
            std::shared_ptr< return_ref > ret =
                std::static_pointer_cast< return_ref >(ref);
            address_set& fptrs = get_address_set(ret->_fptr.str());
            for (address_set::iterator it = fptrs.begin(); it != fptrs.end();
                 ++it) {
              add_address_set(load->_lhs.str(), get_address_set(ret->str(*it)));
              add_offset(load->_lhs.str(), get_offset(ret->str(*it)), op);
            }
            break;
          }
          default: { ikos_unreachable("invalid kind"); }
        }
        break;
      }
      default: { ikos_unreachable("invalid kind"); }
    }
  }

  void step(const binary_op& op) {
    for (std::vector< std::shared_ptr< pta_constraint > >::iterator
             it = this->_csts.begin(),
             et = this->_csts.end();
         it != et;
         ++it) {
      process_constraint(*it, op);
    }
  }

public:
  pta_system() {}

  void print(std::ostream& o) const {
    for (std::vector< std::shared_ptr< pta_constraint > >::const_iterator
             it = this->_csts.begin(),
             et = this->_csts.end();
         it != et;
         ++it) {
      o << **it << std::endl;
    }
  }

  /// \brief Add a pointer constraint
  void operator+=(std::shared_ptr< pta_constraint > cst) {
    this->_csts.push_back(cst);
  }

  /// \brief Solve the constraints system
  void solve(std::size_t widening_threshold = 50,
             std::size_t narrowing_threshold = 1) {
    iteration = 0;
    extrapolate widening_op(widening_threshold);
    do {
      ++iteration;
      change_seen = false;
      step(widening_op);
    } while (change_seen);

    /* FIXME: the current implementation of the narrowing is unsound, skipping
     * See https://babelfish.arc.nasa.gov/jira/projects/IKOS/issues/IKOS-71
    refine narrowing_op;
    iteration = 0;
    for (; iteration < narrowing_threshold; ++iteration) {
      step(narrowing_op);
    }
    */
  }

  pta_info get(pointer_var p) const {
    return std::make_pair(get_address_set(p.str()), get_offset(p.str()));
  }

}; // end class pta_system

std::ostream& operator<<(std::ostream& o, const pta_system& s) {
  s.print(o);
  return o;
}

std::ostream& operator<<(std::ostream& o, const pta_info& info) {
  o << "({";
  for (address_set::const_iterator it = info.first.begin();
       it != info.first.end();) {
    o << *it;
    ++it;
    if (it != info.first.end()) {
      o << "; ";
    }
  }
  o << "}, ";
  o << info.second;
  o << ")";
  return o;
}
}

#endif // IKOS_PTA_HPP
