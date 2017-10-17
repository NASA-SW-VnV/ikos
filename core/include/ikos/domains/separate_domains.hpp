/**************************************************************************/ /**
 *
 * \file
 * \brief Generic implementation of non-relational domains.
 *
 * Author: Arnaud J. Venet
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

#ifndef IKOS_SEPARATE_DOMAINS_HPP
#define IKOS_SEPARATE_DOMAINS_HPP

#include <iostream>

#include <ikos/algorithms/patricia_trees.hpp>
#include <ikos/common/types.hpp>
#include <ikos/domains/abstract_domains_api.hpp>

namespace ikos {

/// \brief Generic implementation of non-relational domains
template < typename Key, typename Value >
class separate_domain : public abstract_domain {
private:
  typedef patricia_tree< Key, Value > patricia_tree_t;
  typedef typename patricia_tree_t::unary_op_t unary_op_t;
  typedef typename patricia_tree_t::binary_op_t binary_op_t;
  typedef typename patricia_tree_t::partial_order_t partial_order_t;

public:
  typedef separate_domain< Key, Value > separate_domain_t;
  typedef typename patricia_tree_t::iterator iterator;

private:
  bool _is_bottom;
  patricia_tree_t _tree;

private:
  class bottom_found {};

  class join_op : public binary_op_t {
    boost::optional< Value > apply(Value x, Value y) {
      Value z = x.operator|(y);
      if (z.is_top()) {
        return boost::optional< Value >();
      } else {
        return boost::optional< Value >(z);
      }
    };

    bool default_is_absorbing() { return true; }

  }; // end class join_op

  class widening_op : public binary_op_t {
    boost::optional< Value > apply(Value x, Value y) {
      Value z = x.operator||(y);
      if (z.is_top()) {
        return boost::optional< Value >();
      } else {
        return boost::optional< Value >(z);
      }
    };

    bool default_is_absorbing() { return true; }

  }; // end class widening_op

  class meet_op : public binary_op_t {
    boost::optional< Value > apply(Value x, Value y) {
      Value z = x.operator&(y);
      if (z.is_bottom()) {
        throw bottom_found();
      } else {
        return boost::optional< Value >(z);
      }
    };

    bool default_is_absorbing() { return false; }

  }; // end class meet_op

  class narrowing_op : public binary_op_t {
    boost::optional< Value > apply(Value x, Value y) {
      Value z = x.operator&&(y);
      if (z.is_bottom()) {
        throw bottom_found();
      } else {
        return boost::optional< Value >(z);
      }
    };

    bool default_is_absorbing() { return false; }

  }; // end class narrowing_op

  class domain_po : public partial_order_t {
    bool leq(Value x, Value y) { return x.operator<=(y); }

    bool default_is_top() { return true; }

  }; // end class domain_po

public:
  static separate_domain_t top() { return separate_domain_t(); }

  static separate_domain_t bottom() { return separate_domain_t(false); }

private:
  static patricia_tree_t apply_operation(binary_op_t& o,
                                         patricia_tree_t t1,
                                         patricia_tree_t t2) {
    t1.merge_with(t2, o);
    return t1;
  }

private:
  separate_domain(patricia_tree_t t) : _is_bottom(false), _tree(t) {}

  separate_domain(bool b) : _is_bottom(!b) {}

public:
  separate_domain() : _is_bottom(false) {}

  separate_domain(const separate_domain_t& e)
      : _is_bottom(e._is_bottom), _tree(e._tree) {}

  separate_domain_t& operator=(separate_domain_t e) {
    this->_is_bottom = e._is_bottom;
    this->_tree = e._tree;
    return *this;
  }

  iterator begin() {
    if (this->is_bottom()) {
      throw logic_error("separate domain: trying to call begin() on bottom");
    } else {
      return this->_tree.begin();
    }
  }

  iterator end() {
    if (this->is_bottom()) {
      throw logic_error("separate domain: trying to call end() on bottom");
    } else {
      return this->_tree.end();
    }
  }

  bool is_bottom() { return this->_is_bottom; }

  bool is_top() { return !this->is_bottom() && this->_tree.empty(); }

  bool operator<=(separate_domain_t e) {
    if (this->is_bottom()) {
      return true;
    } else if (e.is_bottom()) {
      return false;
    } else {
      domain_po po;
      return this->_tree.leq(e._tree, po);
    }
  }

  bool operator==(separate_domain_t e) {
    return this->operator<=(e) && e.operator<=(*this);
  }

  /// \brief Join
  separate_domain_t operator|(separate_domain_t e) {
    if (this->is_bottom()) {
      return e;
    } else if (e.is_bottom()) {
      return *this;
    } else {
      join_op o;
      return separate_domain_t(apply_operation(o, this->_tree, e._tree));
    }
  }

  /// \brief Widening
  separate_domain_t operator||(separate_domain_t e) {
    if (this->is_bottom()) {
      return e;
    } else if (e.is_bottom()) {
      return *this;
    } else {
      widening_op o;
      return separate_domain_t(apply_operation(o, this->_tree, e._tree));
    }
  }

  separate_domain_t join_loop(separate_domain_t e) {
    return this->operator|(e);
  }

  separate_domain_t join_iter(separate_domain_t e) {
    return this->operator|(e);
  }

  /// \brief Meet
  separate_domain_t operator&(separate_domain_t e) {
    if (this->is_bottom() || e.is_bottom()) {
      return this->bottom();
    } else {
      try {
        meet_op o;
        return separate_domain_t(apply_operation(o, this->_tree, e._tree));
      } catch (bottom_found&) {
        return this->bottom();
      }
    }
  }

  /// \brief Narrowing
  separate_domain_t operator&&(separate_domain_t e) {
    if (this->is_bottom() || e.is_bottom()) {
      return separate_domain_t(false);
    } else {
      try {
        narrowing_op o;
        return separate_domain_t(apply_operation(o, this->_tree, e._tree));
      } catch (bottom_found&) {
        return this->bottom();
      }
    }
  }

  void set(Key k, Value v) {
    if (!this->is_bottom()) {
      if (v.is_bottom()) {
        this->_is_bottom = true;
        this->_tree.clear();
      } else if (v.is_top()) {
        this->_tree.remove(k);
      } else {
        this->_tree.insert(k, v);
      }
    }
  }

  void set_to_bottom() {
    this->_is_bottom = true;
    this->_tree = patricia_tree_t();
  }

  separate_domain_t& operator-=(Key k) {
    if (!this->is_bottom()) {
      this->_tree.remove(k);
    }
    return *this;
  }

  Value operator[](Key k) {
    if (this->is_bottom()) {
      return Value::bottom();
    } else {
      boost::optional< Value > v = this->_tree.lookup(k);
      if (v) {
        return *v;
      } else {
        return Value::top();
      }
    }
  }

  void write(std::ostream& o) {
    if (this->is_bottom()) {
      o << "_|_";
    } else {
      o << "{";
      for (typename patricia_tree_t::iterator it = this->_tree.begin();
           it != this->_tree.end();) {
        Key key = it->first;
        index_traits< Key >::write(o, key);
        o << " -> ";
        Value value = it->second;
        value.write(o);
        ++it;
        if (it != this->_tree.end()) {
          o << "; ";
        }
      }
      o << "}";
    }
  }

  static std::string domain_name() { return "Separate"; }

}; // end class separate_domain

} // end namespace ikos

#endif // IKOS_SEPARATE_DOMAINS_HPP
