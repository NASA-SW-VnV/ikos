/*******************************************************************************
 *
 * Abstract domain for nullity analysis.
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

#ifndef IKOS_NULLITY_HPP
#define IKOS_NULLITY_HPP

#include <functional>
#include <iostream>
#include <vector>

#include <ikos/common/types.hpp>
#include <ikos/domains/separate_domains.hpp>

namespace ikos {

// A lattice for nullity
class nullity_value : public writeable {
  typedef enum {
    Bottom = 0x0, /*unused*/
    Null = 0x1,
    NonNull = 0x2,
    MayNull = 0x3
  } kind_t;

  kind_t _value;

  nullity_value(kind_t v) : _value(v){};

public:
  nullity_value() : _value(MayNull) {}

public:
  static nullity_value bottom() { return nullity_value(Bottom); }
  static nullity_value top() { return nullity_value(MayNull); }

  static nullity_value non_null() { return nullity_value(NonNull); }
  static nullity_value null() { return nullity_value(Null); }

public:
  nullity_value(const nullity_value& other)
      : writeable(), _value(other._value) {}

  nullity_value& operator=(nullity_value other) {
    this->_value = other._value;
    return *this;
  }

  bool is_bottom() { return (this->_value == Bottom); }
  bool is_top() { return (this->_value == MayNull); }
  bool is_non_null() const { return (this->_value == NonNull); }
  bool is_null() const { return (this->_value == Null); }

  bool operator<=(nullity_value other) {
    if (this->_value == Bottom)
      return true;
    else if (this->_value == MayNull)
      return (other._value == MayNull);
    else if (this->_value == NonNull)
      return ((other._value == this->_value) || (other._value == MayNull));
    else if (this->_value == Null)
      return ((other._value == this->_value) || (other._value == MayNull));

    assert(false && "unreachable");
    return false;
  }

  bool operator==(nullity_value other) {
    return (this->_value == other._value);
  }

  nullity_value operator|(nullity_value other) {
    return nullity_value(static_cast< kind_t >(
        static_cast< int >(this->_value) | static_cast< int >(other._value)));
  }

  nullity_value operator||(nullity_value other) {
    return this->operator|(other);
  }

  nullity_value operator&(nullity_value other) {
    return nullity_value(static_cast< kind_t >(
        static_cast< int >(this->_value) & static_cast< int >(other._value)));
  }

  nullity_value operator&&(nullity_value other) {
    return this->operator&(other);
  }

  void write(std::ostream& o) {
    switch (this->_value) {
      case Bottom: {
        o << "_|_";
        break;
      }
      case MayNull: {
        o << "T";
        break;
      }
      case NonNull: {
        o << "NN";
        break;
      }
      case Null: {
        o << "N";
        break;
      }
    }
  }
}; // end class nullity_value

// An abstract domain for nullity
template < typename VariableName >
class nullity_domain : public writeable {
  typedef separate_domain< VariableName, nullity_value > separate_domain_t;

public:
  typedef nullity_domain< VariableName > nullity_domain_t;

  typedef typename separate_domain_t::iterator iterator;

private:
  separate_domain_t _env;

  nullity_domain(separate_domain_t env) : _env(env) {}

  struct toNullity : public std::unary_function< VariableName, nullity_value > {
    separate_domain_t _env;
    toNullity(separate_domain_t env) : _env(env) {}
    nullity_value operator()(VariableName v) { return _env[v]; }
  };

  void assign(VariableName x, std::vector< nullity_value > ys) {
    if (is_bottom())
      return;

    bool all_nonnull = true;
    bool some_null = false;
    for (unsigned int i = 0; i < ys.size(); i++) {
      nullity_value abs_val = ys[i];
      all_nonnull &= abs_val.is_non_null();
      some_null |= abs_val.is_null();
    }
    if (all_nonnull)
      this->_env.set(x, nullity_value::non_null());
    else if (some_null)
      this->_env.set(x, nullity_value::null());
    else
      this->_env.set(x, nullity_value::top());
  }

  void apply(VariableName x, nullity_value y, nullity_value z) {
    if (is_bottom())
      return;

    if (y.is_null() || z.is_null())
      this->_env.set(x, nullity_value::null());
    else
      this->_env.set(x, y | z);
  }

public:
  static nullity_domain_t top() {
    return nullity_domain(separate_domain_t::top());
  }

  static nullity_domain_t bottom() {
    return nullity_domain(separate_domain_t::bottom());
  }

public:
  nullity_domain() : _env(separate_domain_t::top()) {}

  nullity_domain(const nullity_domain_t& e) : writeable(), _env(e._env) {}

  nullity_domain_t& operator=(nullity_domain_t e) {
    this->_env = e._env;
    return *this;
  }

  iterator begin() { return this->_env.begin(); }

  iterator end() { return this->_env.end(); }

  bool is_bottom() { return this->_env.is_bottom(); }

  bool is_top() { return this->_env.is_top(); }

  bool operator<=(nullity_domain_t e) { return (this->_env <= e._env); }

  bool operator==(nullity_domain_t e) {
    return ((this->_env <= e._env) && (e._env <= this->_env));
  }

  nullity_domain_t operator|(nullity_domain_t e) {
    return (this->_env | e._env);
  }

  nullity_domain_t operator&(nullity_domain_t e) {
    return (this->_env & e._env);
  }

  nullity_domain_t operator||(nullity_domain_t e) {
    return (this->_env || e._env);
  }

  nullity_domain_t operator&&(nullity_domain_t e) {
    return (this->_env && e._env);
  }

  void set(VariableName v, nullity_value e) {
    if (is_bottom())
      return;

    this->_env.set(v, e);
  }

  void assign(VariableName x, std::vector< VariableName > in) {
    if (is_bottom())
      return;

    std::vector< nullity_value > out;
    toNullity f(this->_env);
    transform(in.begin(), in.end(), back_inserter(out), f);
    this->assign(x, out);
  }

  void assign(VariableName x, VariableName y) {
    if (is_bottom())
      return;

    this->_env.set(x, this->_env[y]);
  }

  void operator-=(VariableName v) { this->_env -= v; }

  nullity_value operator[](VariableName v) { return this->_env[v]; }

  void assertion(bool IsEqual, VariableName x, VariableName y) {
    if (IsEqual) { // x==y
      nullity_value z = this->_env[x] & this->_env[y];
      this->_env.set(x, z);
      this->_env.set(y, z);
    } else { // x!=y
      if (this->_env[x].is_null() && this->_env[y].is_null()) {
        *this = bottom();
      } else if (this->_env[x].is_top() && this->_env[y].is_null()) {
        this->_env.set(x, this->_env[x] & nullity_value::non_null());
      } else if (this->_env[y].is_top() && this->_env[x].is_null()) {
        this->_env.set(y, this->_env[y] & nullity_value::non_null());
      }
    }
  }

  void assertion(bool IsEqual, VariableName x, nullity_value nv) {
    if (IsEqual) { // x==y
      this->_env.set(x, this->_env[x] & nv);
    } else { // x!=y
      if (this->_env[x].is_null() && nv.is_null()) {
        *this = bottom();
      } else if (this->_env[x].is_top() && nv.is_null()) {
        this->_env.set(x, this->_env[x] & nullity_value::non_null());
      }
    }
  }

  void write(std::ostream& o) { this->_env.write(o); }

}; // class nullity_domain

} // end ikos namespace

#endif // IKOS_NULLITY_HPP
