/*******************************************************************************
 *
 * Abstract domain for uninitialized variables with arrays.
 *
 * Author: Jorge A. Navas
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2019 United States Government as represented by the
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

#pragma once

#include <ikos/core/common/type.hpp>
#include <ikos/core/domain/array_graph.hpp>
#include <ikos/core/domain/domain_product.hpp>
#include <ikos/core/domain/uninitialized.hpp>

namespace ikos {
namespace core {

// An abstract domain for reasoning about uninitialized variables
// with arrays
template < typename NumDomain,
           typename Number,
           typename VariableName,
           typename VariableFactory >
class uninitialized_array_domain final : public abstract_domain {
private:
  typedef uninitialized_array_domain< NumDomain,
                                      Number,
                                      VariableName,
                                      VariableFactory >
      uninitialized_array_domain_t;

private:
  typedef uninitialized_domain_impl< VariableName > uva_scalar_domain_t;

private:
  typedef array_graph_domain< NumDomain,
                              Number,
                              VariableName,
                              uva_scalar_domain_t,
                              VariableFactory >
      uva_array_domain_t;

private:
  typedef typename uva_array_domain_t::linear_constraint_t linear_constraint_t;
  typedef typename uva_array_domain_t::linear_constraint_system_t
      linear_constraint_system_t;
  typedef typename uva_array_domain_t::linear_expression_t linear_expression_t;
  typedef typename uva_array_domain_t::variable_t variable_t;

private:
  typedef domain_product2< uva_scalar_domain_t, uva_array_domain_t >
      uva_product_t;

private:
  uva_product_t _inv;

private:
  // Helpers
  void assign_scalar(VariableName x, const linear_expression_t& e) {
    if (e.is_constant()) {
      this->_inv.first().set(x, uninitialized_value::initialized());
    } else {
      uninitialized_value result = uninitialized_value::initialized();
      for (auto it = e.begin(), et = e.end(); it != et; ++it) {
        uninitialized_value v = this->_inv.first()[it->first];
        if (v.is_uninitialized()) {
          result = uninitialized_value::uninitialized();
          break;
        } else {
          result = result | v;
        }
      }
      this->_inv.first().set(x, result);
    }
  }

  uva_scalar_domain_t eval_scalar(VariableName x,
                                  const linear_expression_t& e) {
    uva_scalar_domain_t out = uva_scalar_domain_t::top();
    if (e.is_constant()) {
      out.set(x, uninitialized_value::initialized());
    } else {
      uninitialized_value result = uninitialized_value::initialized();
      for (auto it = e.begin(), et = e.end(); it != et; ++it) {
        uninitialized_value v = this->_inv.first()[it->first];
        if (v.is_uninitialized()) {
          result = uninitialized_value::uninitialized();
          break;
        } else {
          result = result | v;
        }
      }
      out.set(x, result);
    }
    return out;
  }

private:
  uninitialized_array_domain(uva_product_t inv) : _inv(inv) {}

public:
  static uninitialized_array_domain_t top() {
    return uninitialized_array_domain(uva_product_t::top());
  }

  static uninitialized_array_domain_t bottom() {
    return uninitialized_array_domain(uva_product_t::bottom());
  }

public:
  uva_scalar_domain_t& scalars() { return this->_inv.first(); }

  uva_array_domain_t& arrays() { return this->_inv.second(); }

public:
  uninitialized_array_domain() : _inv(uva_product_t::top()) {}

  uninitialized_array_domain(const uninitialized_array_domain_t& other)
      : _inv(other._inv) {}

  uninitialized_array_domain_t& operator=(uninitialized_array_domain_t other) {
    this->_inv = other._inv;
    return *this;
  }

  bool is_bottom() { return this->_inv.is_bottom(); }

  bool is_top() { return this->_inv.is_top(); }

  bool operator<=(uninitialized_array_domain_t other) {
    return (this->_inv <= other._inv);
  }

  bool operator==(uninitialized_array_domain_t other) {
    return (this->_inv == other._inv);
  }

  uninitialized_array_domain_t operator|(uninitialized_array_domain_t other) {
    return (this->_inv | other._inv);
  }

  uninitialized_array_domain_t operator||(uninitialized_array_domain_t other) {
    return (this->_inv || other._inv);
  }

  uninitialized_array_domain_t operator&(uninitialized_array_domain_t other) {
    return (this->_inv & other._inv);
  }

  uninitialized_array_domain_t operator&&(uninitialized_array_domain_t other) {
    return (this->_inv && other._inv);
  }

  uninitialized_value operator[](VariableName v) {
    return this->_inv.first()[v];
  }

  void operator-=(VariableName v) {
    this->_inv.first() -= v;
    this->_inv.second() -= v;
  }

  /// Transfer functions

  // for scalar variables
  void set(VariableName v, uninitialized_value e) {
    this->_inv.first().set(v, e);
  }

  void assertion(const linear_constraint_t& cst, VariableFactory& vfac) {
    // nothing to do in the scalar domain
    this->_inv.second().assertion(cst, vfac);
  }

  void assertion(const linear_constraint_system_t& csts,
                 VariableFactory& vfac) {
    // nothing to do in the scalar domain
    this->_inv.second().assertion(csts, vfac);
  }

  void apply(operation_t op,
             VariableName x,
             VariableName y,
             VariableName z,
             VariableFactory& vfac) {
    this->_inv.first().assign_uninitialized(x, y, z);
    this->_inv.second().apply(op, x, y, z, vfac);
  }

  void assign(VariableName x,
              const linear_expression_t& e,
              VariableFactory& vfac) {
    assign_scalar(x, e);
    this->_inv.second().assign(x, e, vfac);
  }

  void apply(operation_t op,
             VariableName x,
             VariableName y,
             Number z,
             VariableFactory& vfac) {
    this->_inv.first().assign_uninitialized(x, y);
    this->_inv.second().apply(op, x, y, z, vfac);
  }

  // x += z
  void apply(operation_t op, VariableName x, Number z, VariableFactory& vfac) {
    this->_inv.second().apply(op, x, z, vfac);
  }

  void array_write(VariableName a,
                   VariableName i,
                   const linear_expression_t& e) {
    // reduction step: we update the array graph using the scalar component
    uva_scalar_domain_t w = eval_scalar(a, e);
    this->_inv.second().store(i, w);
  }

  void array_read(VariableName x, VariableName a, VariableName i) {
    // reduction step: we update the scalar component from the array graph.
    uva_scalar_domain_t values = this->_inv.second()[i];
    uninitialized_value val_a = values[a];
    this->set(x, val_a);
  }

  void write(std::ostream& o) {
#if 0
    o << this->_inv;
#else
    // less verbose: only the scalar variables
    o << this->_inv.first();
#endif
  }

  static std::string domain_name() {
    return "Uninitialized Array of " + NumDomain::domain_name();
  }

}; // end class uninitialized_array_domain

} // end namespace core
} // end namespace ikos
