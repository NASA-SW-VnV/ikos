/**************************************************************************/ /**
 *
 * \file
 * \brief Implementation of an abstract domain for (un)initialized variables.
 *
 * Author: Jorge A. Navas
 *
 * Contributors: Maxime Arthaud
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

#ifndef IKOS_UNINITIALIZED_HPP
#define IKOS_UNINITIALIZED_HPP

#include <ikos/common/types.hpp>
#include <ikos/domains/abstract_domains_api.hpp>
#include <ikos/domains/separate_domains.hpp>
#include <ikos/domains/uninitialized_domains_api.hpp>

namespace ikos {

/// \brief Uninitialized abstract value
///
/// This is either top, bottom, initialized, uninitialized.
class uninitialized_value : public writeable {
private:
  typedef enum {
    Bottom = 0x0, /* unused */
    Initialized = 0x1,
    Uninitialized = 0x2,
    MayUninitialized = 0x3
  } kind_t;

private:
  kind_t _value;

private:
  uninitialized_value(kind_t v) : _value(v) {}

public:
  static uninitialized_value bottom() { return uninitialized_value(Bottom); }

  static uninitialized_value top() {
    return uninitialized_value(MayUninitialized);
  }

  static uninitialized_value initialized() {
    return uninitialized_value(Initialized);
  }

  static uninitialized_value uninitialized() {
    return uninitialized_value(Uninitialized);
  }

public:
  uninitialized_value() : _value(MayUninitialized) {}

  uninitialized_value(const uninitialized_value& other)
      : _value(other._value) {}

  uninitialized_value& operator=(uninitialized_value other) {
    this->_value = other._value;
    return *this;
  }

  bool is_bottom() { return (this->_value == Bottom); }

  bool is_top() { return (this->_value == MayUninitialized); }

  bool is_initialized() const { return (this->_value == Initialized); }

  bool is_uninitialized() const { return (this->_value == Uninitialized); }

  bool operator<=(uninitialized_value other) {
    if (this->_value == Bottom) {
      return true;
    } else if (this->_value == MayUninitialized) {
      return (other._value == MayUninitialized);
    } else if (this->_value == Initialized) {
      return ((other._value == this->_value) ||
              (other._value == MayUninitialized));
    } else if (this->_value == Uninitialized) {
      return ((other._value == this->_value) ||
              (other._value == MayUninitialized));
    } else {
      ikos_unreachable("unreachable");
    }
  }

  bool operator==(uninitialized_value other) {
    return (this->_value == other._value);
  }

  uninitialized_value operator|(uninitialized_value other) {
    return uninitialized_value(static_cast< kind_t >(
        static_cast< int >(this->_value) | static_cast< int >(other._value)));
  }

  uninitialized_value operator||(uninitialized_value other) {
    return this->operator|(other);
  }

  uninitialized_value operator&(uninitialized_value other) {
    return uninitialized_value(static_cast< kind_t >(
        static_cast< int >(this->_value) & static_cast< int >(other._value)));
  }

  uninitialized_value operator&&(uninitialized_value other) {
    return this->operator&(other);
  }

  void write(std::ostream& o) {
    switch (this->_value) {
      case Bottom: {
        o << "_|_";
        break;
      }
      case MayUninitialized: {
        o << "T";
        break;
      }
      case Initialized: {
        o << "I";
        break;
      }
      case Uninitialized: {
        o << "U";
        break;
      }
    }
  }

}; // end class uninitialized_value

/// \brief Abstract domain to reason about uninitialized variables
template < typename VariableName >
class uninitialized_domain_impl : public abstract_domain,
                                  public uninitialized_domain< VariableName > {
  template < typename Any1, typename Any2, typename Any3 >
  friend class uninitialized_array_domain;

private:
  typedef separate_domain< VariableName, uninitialized_value >
      separate_domain_t;

public:
  typedef typename separate_domain_t::iterator iterator;
  typedef uninitialized_domain_impl< VariableName > uninitialized_domain_t;

private:
  separate_domain_t _env;

private:
  uninitialized_domain_impl(separate_domain_t env) : _env(env) {}

public:
  static uninitialized_domain_t top() {
    return uninitialized_domain_impl(separate_domain_t::top());
  }

  static uninitialized_domain_t bottom() {
    return uninitialized_domain_impl(separate_domain_t::bottom());
  }

public:
  uninitialized_domain_impl() : _env(separate_domain_t::top()) {}

  uninitialized_domain_impl(const uninitialized_domain_t& e) : _env(e._env) {}

  uninitialized_domain_t& operator=(uninitialized_domain_t e) {
    this->_env = e._env;
    return *this;
  }

  iterator begin() { return this->_env.begin(); }

  iterator end() { return this->_env.end(); }

  bool is_bottom() { return this->_env.is_bottom(); }

  bool is_top() { return this->_env.is_top(); }

  bool operator<=(uninitialized_domain_t e) { return this->_env <= e._env; }

  bool operator==(uninitialized_domain_t e) {
    return this->_env <= e._env && e._env <= this->_env;
  }

  uninitialized_domain_t operator|(uninitialized_domain_t e) {
    return this->_env | e._env;
  }

  uninitialized_domain_t operator||(uninitialized_domain_t e) {
    return this->_env || e._env;
  }

  uninitialized_domain_t join_loop(uninitialized_domain_t e) {
    return this->operator|(e);
  }

  uninitialized_domain_t join_iter(uninitialized_domain_t e) {
    return this->operator|(e);
  }

  uninitialized_domain_t operator&(uninitialized_domain_t e) {
    return this->_env & e._env;
  }

  uninitialized_domain_t operator&&(uninitialized_domain_t e) {
    return this->_env && e._env;
  }

  void set(VariableName v, uninitialized_value e) {
    if (is_bottom())
      return;

    this->_env.set(v, e);
  }

  void make_initialized(VariableName v) {
    this->set(v, uninitialized_value::initialized());
  }

  void make_uninitialized(VariableName v) {
    this->set(v, uninitialized_value::uninitialized());
  }

  uninitialized_value operator[](VariableName v) { return this->_env[v]; }

  bool is_initialized(VariableName v) {
    if (is_bottom()) {
      throw logic_error(
          "uninitialized domain: trying to call is_initialized() on bottom");
    } else {
      return this->_env[v].is_initialized();
    }
  }

  bool is_uninitialized(VariableName v) {
    if (is_bottom()) {
      throw logic_error(
          "uninitialized domain: trying to call is_uninitialized() on bottom");
    } else {
      return this->_env[v].is_uninitialized();
    }
  }

  void assign_uninitialized(VariableName x, VariableName y) {
    if (is_bottom())
      return;

    this->_env.set(x, this->_env[y]);
  }

  void assign_uninitialized(VariableName x, VariableName y, VariableName z) {
    if (is_bottom())
      return;

    this->assign_uninitialized(x, this->_env[y], this->_env[z]);
  }

  void assign_uninitialized(VariableName x,
                            uninitialized_value y,
                            uninitialized_value z) {
    if (is_bottom())
      return;

    if (y.is_uninitialized() || z.is_uninitialized()) {
      this->_env.set(x, uninitialized_value::uninitialized());
    } else {
      this->_env.set(x, y | z);
    }
  }

  void operator-=(VariableName v) { forget_uninitialized(v); }

  void forget(VariableName v) { forget_uninitialized(v); }

  void forget_uninitialized(VariableName v) { this->_env -= v; }

  template < typename Iterator >
  void forget(Iterator begin, Iterator end) {
    forget_uninitialized(begin, end);
  }

  template < typename Iterator >
  void forget_uninitialized(Iterator begin, Iterator end) {
    for (auto it = begin; it != end; ++it) {
      this->_env -= *it;
    }
  }

  void write(std::ostream& o) { this->_env.write(o); }

  static std::string domain_name() { return "Uninitialized"; }

}; // end class uninitialized_domain_impl

} // end ikos namespace

#endif // IKOS_UNINITIALIZED_HPP
