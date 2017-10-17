/**************************************************************************/ /**
 *
 * \file
 * \brief An implementation of discrete domains based on Patricia trees.
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

#ifndef IKOS_DISCRETE_DOMAINS_HPP
#define IKOS_DISCRETE_DOMAINS_HPP

#include <iostream>

#include <ikos/algorithms/patricia_trees.hpp>
#include <ikos/common/types.hpp>
#include <ikos/domains/separate_domains.hpp>
#include <ikos/domains/abstract_domains_api.hpp>

namespace ikos {

/// \brief Generic discrete abstract domain.
///
/// The implementation is based on patricia trees.
template < typename Element >
class discrete_domain : public abstract_domain {
private:
  typedef patricia_tree_set< Element > ptset_t;

public:
  typedef discrete_domain< Element > discrete_domain_t;
  typedef collection< Element > collection_t;
  typedef typename ptset_t::iterator iterator;

private:
  bool _is_top;
  ptset_t _set;

private:
  discrete_domain(bool is_top) : _is_top(is_top) {}

  discrete_domain(ptset_t set) : _is_top(false), _set(set) {}

public:
  static discrete_domain_t bottom() { return discrete_domain_t(false); }

  static discrete_domain_t top() { return discrete_domain_t(true); }

public:
  discrete_domain() : _is_top(true) {}

  discrete_domain(const discrete_domain_t& other)
      : _is_top(other._is_top), _set(other._set) {}

  discrete_domain(Element s) : _is_top(false), _set(s) {}

  discrete_domain(collection_t c) : _is_top(false) {
    for (typename collection_t::iterator it = c.begin(); it != c.end(); ++it) {
      this->_set += *it;
    }
  }

  bool is_top() { return this->_is_top; }

  bool is_bottom() { return (!this->_is_top && this->_set.empty()); }

  bool operator<=(discrete_domain_t other) {
    return other._is_top || (!this->_is_top && this->_set <= other._set);
  }

  bool operator==(discrete_domain_t other) {
    return (this->_is_top && other._is_top) || (this->_set == other._set);
  }

  discrete_domain_t operator|(discrete_domain_t other) {
    if (this->_is_top || other._is_top) {
      return discrete_domain_t(true);
    } else {
      return discrete_domain_t(this->_set | other._set);
    }
  }

  discrete_domain_t operator||(discrete_domain_t other) {
    return this->operator|(other);
  }

  discrete_domain_t join_loop(discrete_domain_t other) {
    return this->operator|(other);
  }

  discrete_domain_t join_iter(discrete_domain_t other) {
    return this->operator|(other);
  }

  discrete_domain_t operator&(discrete_domain_t other) {
    if (this->is_bottom() || other.is_bottom()) {
      return discrete_domain_t(false);
    } else if (this->_is_top) {
      return other;
    } else if (other._is_top) {
      return *this;
    } else {
      return discrete_domain_t(this->_set & other._set);
    }
  }

  discrete_domain_t operator&&(discrete_domain_t other) {
    return this->operator&(other);
  }

  discrete_domain_t& operator+=(Element s) {
    if (!this->_is_top) {
      this->_set += s;
    }
    return *this;
  }

  discrete_domain_t& operator+=(collection_t c) {
    if (!this->_is_top) {
      for (typename collection_t::iterator it = c.begin(); it != c.end();
           ++it) {
        this->_set += *it;
      }
    }
    return *this;
  }

  discrete_domain_t operator+(Element s) {
    discrete_domain_t r(*this);
    r.operator+=(s);
    return r;
  }

  discrete_domain_t operator+(collection_t c) {
    discrete_domain_t r(*this);
    r.operator+=(c);
    return r;
  }

  discrete_domain_t& operator-=(Element s) {
    if (!this->_is_top) {
      this->_set -= s;
    }
    return *this;
  }

  discrete_domain_t& operator-=(collection_t c) {
    if (!this->_is_top) {
      for (typename collection_t::iterator it = c.begin(); it != c.end();
           ++it) {
        this->_set -= *it;
      }
    }
    return *this;
  }

  discrete_domain_t operator-(Element s) {
    discrete_domain_t r(*this);
    r.operator-=(s);
    return r;
  }

  discrete_domain_t operator-(collection_t c) {
    discrete_domain_t r(*this);
    r.operator-=(c);
    return r;
  }

  std::size_t size() {
    if (this->_is_top) {
      throw logic_error("discrete domain: trying to call size() on top");
    } else {
      return this->_set.size();
    }
  }

  iterator begin() {
    if (this->_is_top) {
      throw logic_error("discrete domain: trying to call begin() on top");
    } else {
      return this->_set.begin();
    }
  }

  iterator end() {
    if (this->_is_top) {
      throw logic_error("discrete domain: trying to call end() on top");
    } else {
      return this->_set.end();
    }
  }

  void write(std::ostream& o) {
    if (this->_is_top) {
      o << "{...}";
    } else if (this->_set.empty()) {
      o << "_|_";
    } else {
      o << this->_set;
    }
  }

  static std::string domain_name() { return "Discrete"; }

}; // end class discrete_domain

} // end namespace ikos

#endif // IKOS_DISCRETE_DOMAINS_HPP
