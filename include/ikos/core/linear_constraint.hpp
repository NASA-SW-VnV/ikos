/*******************************************************************************
 *
 * \file
 * \brief Data structures for the symbolic manipulation of linear constraints.
 *
 * Author: Arnaud J. Venet
 *
 * Contributors: Maxime Arthaud
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

#include <algorithm>
#include <vector>

#include <ikos/core/adt/patricia_tree/set.hpp>
#include <ikos/core/linear_expression.hpp>
#include <ikos/core/support/assert.hpp>

namespace ikos {
namespace core {

/// \brief Represents a linear constraint
template < typename Number, typename VariableRef >
class LinearConstraint {
public:
  using VariableExpressionT = VariableExpression< Number, VariableRef >;
  using LinearExpressionT = LinearExpression< Number, VariableRef >;

public:
  /// \brief Kind of linear constraint
  enum Kind {
    Equality,    // ==
    Disequation, // !=
    Inequality,  // <=
  };

private:
  LinearExpressionT _expr;
  Kind _kind;

public:
  /// \brief No default constructor
  LinearConstraint() = delete;

  /// \brief Constructor
  LinearConstraint(LinearExpressionT expr, Kind kind)
      : _expr(std::move(expr)), _kind(kind) {}

  /// \brief Copy constructor
  LinearConstraint(const LinearConstraint&) = default;

  /// \brief Move constructor
  LinearConstraint(LinearConstraint&&) = default;

  /// \brief Copy assignment operator
  LinearConstraint& operator=(const LinearConstraint&) = default;

  /// \brief Move assignment operator
  LinearConstraint& operator=(LinearConstraint&&) = default;

  /// \brief Destructor
  ~LinearConstraint() = default;

  /// \brief Create the tautology 0 == 0
  static LinearConstraint tautology() {
    return LinearConstraint(LinearExpressionT(), Equality);
  }

  /// \brief Create the contradiction 0 != 0
  static LinearConstraint contradiction() {
    return LinearConstraint(LinearExpressionT(), Disequation);
  }

  /// \brief Return true if the constraint is a tautology
  bool is_tautology() const {
    if (!this->_expr.is_constant()) {
      return false;
    }

    switch (this->_kind) {
      case Equality: {
        // x == 0
        return this->_expr.constant() == 0;
      }
      case Disequation: {
        // x != 0
        return this->_expr.constant() != 0;
      }
      case Inequality: {
        // x <= 0
        return this->_expr.constant() <= 0;
      }
      default: {
        ikos_unreachable("unexpected kind");
      }
    }
  }

  /// \brief Return true if the constraint is a contradiction
  bool is_contradiction() const {
    if (!this->_expr.is_constant()) {
      return false;
    }

    switch (this->_kind) {
      case Equality: {
        // x == 0
        return this->_expr.constant() != 0;
      }
      case Disequation: {
        // x != 0
        return this->_expr.constant() == 0;
      }
      case Inequality: {
        // x <= 0
        return this->_expr.constant() > 0;
      }
      default: {
        ikos_unreachable("unexpected kind");
      }
    }
  }

  /// \brief Return true if the constraint is an equality (==)
  bool is_equality() const { return this->_kind == Equality; }

  /// \brief Return true if the constraint is an disequation (!=)
  bool is_disequation() const { return this->_kind == Disequation; }

  /// \brief Return true if the constraint is an inequality (<=)
  bool is_inequality() const { return this->_kind == Inequality; }

  /// \brief Retun the linear expression
  const LinearExpressionT& expression() const { return this->_expr; }

  /// \brief Return the kind
  Kind kind() const { return this->_kind; }

  /// \brief Return the begin iterator over the terms
  typename LinearExpressionT::TermIterator begin() {
    return this->_expr.begin();
  }
  typename LinearExpressionT::TermConstIterator begin() const {
    return this->_expr.begin();
  }
  typename LinearExpressionT::TermConstIterator cbegin() const {
    return this->_expr.cbegin();
  }

  /// \brief Return the end iterator over the terms
  typename LinearExpressionT::TermIterator end() { return this->_expr.end(); }
  typename LinearExpressionT::TermConstIterator end() const {
    return this->_expr.end();
  }
  typename LinearExpressionT::TermConstIterator cend() const {
    return this->_expr.cend();
  }

  /// \brief Return the constant
  Number constant() const { return -this->_expr.constant(); }

  /// \brief Return the number of terms
  std::size_t num_terms() const { return this->_expr.num_terms(); }

  /// \brief Return the factor for the given variable
  Number factor(VariableRef var) const { return this->_expr.factor(var); }

  /// \brief Return the set of variables present in the linear constraint
  PatriciaTreeSet< VariableRef > variables() const {
    return this->_expr.variables();
  }

  /// \brief Dump the linear constraint, for debugging purpose
  void dump(std::ostream& o) const {
    if (this->is_contradiction()) {
      o << "false";
    } else if (this->is_tautology()) {
      o << "true";
    } else {
      LinearExpressionT e = this->_expr - this->_expr.constant();
      Number c = -this->_expr.constant();

      e.dump(o);
      switch (this->_kind) {
        case Equality: {
          o << " = ";
          break;
        }
        case Disequation: {
          o << " != ";
          break;
        }
        case Inequality: {
          o << " <= ";
          break;
        }
        default: {
          ikos_unreachable("unexpected kind");
        }
      }
      o << c;
    }
  }

}; // end class LinearConstraint

/// \brief Write a linear constraint on a stream
template < typename Number, typename VariableRef >
inline std::ostream& operator<<(
    std::ostream& o, const LinearConstraint< Number, VariableRef >& e) {
  e.dump(o);
  return o;
}

/// \name Operator <=
/// @{

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator<=(
    LinearExpression< Number, VariableRef > e, const Number& n) {
  return LinearConstraint<
      Number,
      VariableRef >(std::move(e) - n,
                    LinearConstraint< Number, VariableRef >::Inequality);
}

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator<=(
    LinearExpression< Number, VariableRef > e, int n) {
  return LinearConstraint<
      Number,
      VariableRef >(std::move(e) - n,
                    LinearConstraint< Number, VariableRef >::Inequality);
}

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator<=(
    LinearExpression< Number, VariableRef > x,
    VariableExpression< Number, VariableRef > y) {
  return LinearConstraint<
      Number,
      VariableRef >(std::move(x) - std::move(y),
                    LinearConstraint< Number, VariableRef >::Inequality);
}

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator<=(
    VariableExpression< Number, VariableRef > x, const Number& n) {
  return LinearConstraint<
      Number,
      VariableRef >(std::move(x) - n,
                    LinearConstraint< Number, VariableRef >::Inequality);
}

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator<=(
    VariableExpression< Number, VariableRef > x, int n) {
  return LinearConstraint<
      Number,
      VariableRef >(std::move(x) - n,
                    LinearConstraint< Number, VariableRef >::Inequality);
}

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator<=(
    VariableExpression< Number, VariableRef > x,
    VariableExpression< Number, VariableRef > y) {
  return LinearConstraint<
      Number,
      VariableRef >(std::move(x) - std::move(y),
                    LinearConstraint< Number, VariableRef >::Inequality);
}

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator<=(
    VariableExpression< Number, VariableRef > x,
    LinearExpression< Number, VariableRef > y) {
  return LinearConstraint<
      Number,
      VariableRef >(std::move(x) - std::move(y),
                    LinearConstraint< Number, VariableRef >::Inequality);
}

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator<=(
    LinearExpression< Number, VariableRef > x,
    const LinearExpression< Number, VariableRef >& y) {
  return LinearConstraint<
      Number,
      VariableRef >(std::move(x) - y,
                    LinearConstraint< Number, VariableRef >::Inequality);
}

/// @}
/// \name Operator >=
/// @{

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator>=(
    LinearExpression< Number, VariableRef > e, const Number& n) {
  return LinearConstraint<
      Number,
      VariableRef >(n - std::move(e),
                    LinearConstraint< Number, VariableRef >::Inequality);
}

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator>=(
    LinearExpression< Number, VariableRef > e, int n) {
  return LinearConstraint<
      Number,
      VariableRef >(n - std::move(e),
                    LinearConstraint< Number, VariableRef >::Inequality);
}

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator>=(
    LinearExpression< Number, VariableRef > x,
    VariableExpression< Number, VariableRef > y) {
  return LinearConstraint<
      Number,
      VariableRef >(std::move(y) - std::move(x),
                    LinearConstraint< Number, VariableRef >::Inequality);
}

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator>=(
    VariableExpression< Number, VariableRef > x, const Number& n) {
  return LinearConstraint<
      Number,
      VariableRef >(n - std::move(x),
                    LinearConstraint< Number, VariableRef >::Inequality);
}

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator>=(
    VariableExpression< Number, VariableRef > x, int n) {
  return LinearConstraint<
      Number,
      VariableRef >(n - std::move(x),
                    LinearConstraint< Number, VariableRef >::Inequality);
}

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator>=(
    VariableExpression< Number, VariableRef > x,
    VariableExpression< Number, VariableRef > y) {
  return LinearConstraint<
      Number,
      VariableRef >(std::move(y) - std::move(x),
                    LinearConstraint< Number, VariableRef >::Inequality);
}

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator>=(
    VariableExpression< Number, VariableRef > x,
    LinearExpression< Number, VariableRef > y) {
  return LinearConstraint<
      Number,
      VariableRef >(std::move(y) - std::move(x),
                    LinearConstraint< Number, VariableRef >::Inequality);
}

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator>=(
    const LinearExpression< Number, VariableRef >& x,
    LinearExpression< Number, VariableRef > y) {
  return LinearConstraint<
      Number,
      VariableRef >(std::move(y) - x,
                    LinearConstraint< Number, VariableRef >::Inequality);
}

/// @}
/// \name Operator ==
/// @{

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator==(
    LinearExpression< Number, VariableRef > e, const Number& n) {
  return LinearConstraint<
      Number,
      VariableRef >(std::move(e) - n,
                    LinearConstraint< Number, VariableRef >::Equality);
}

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator==(
    LinearExpression< Number, VariableRef > e, int n) {
  return LinearConstraint<
      Number,
      VariableRef >(std::move(e) - n,
                    LinearConstraint< Number, VariableRef >::Equality);
}

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator==(
    LinearExpression< Number, VariableRef > x,
    VariableExpression< Number, VariableRef > y) {
  return LinearConstraint<
      Number,
      VariableRef >(std::move(x) - std::move(y),
                    LinearConstraint< Number, VariableRef >::Equality);
}

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator==(
    VariableExpression< Number, VariableRef > x, const Number& n) {
  return LinearConstraint<
      Number,
      VariableRef >(std::move(x) - n,
                    LinearConstraint< Number, VariableRef >::Equality);
}

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator==(
    VariableExpression< Number, VariableRef > x, int n) {
  return LinearConstraint<
      Number,
      VariableRef >(std::move(x) - n,
                    LinearConstraint< Number, VariableRef >::Equality);
}

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator==(
    VariableExpression< Number, VariableRef > x,
    VariableExpression< Number, VariableRef > y) {
  return LinearConstraint<
      Number,
      VariableRef >(std::move(x) - std::move(y),
                    LinearConstraint< Number, VariableRef >::Equality);
}

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator==(
    VariableExpression< Number, VariableRef > x,
    LinearExpression< Number, VariableRef > y) {
  return LinearConstraint<
      Number,
      VariableRef >(std::move(y) - std::move(x),
                    LinearConstraint< Number, VariableRef >::Equality);
}

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator==(
    LinearExpression< Number, VariableRef > x,
    const LinearExpression< Number, VariableRef >& y) {
  return LinearConstraint<
      Number,
      VariableRef >(std::move(x) - y,
                    LinearConstraint< Number, VariableRef >::Equality);
}

/// @}
/// \name Operator !=
/// @{

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator!=(
    LinearExpression< Number, VariableRef > e, const Number& n) {
  return LinearConstraint<
      Number,
      VariableRef >(std::move(e) - n,
                    LinearConstraint< Number, VariableRef >::Disequation);
}

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator!=(
    LinearExpression< Number, VariableRef > e, int n) {
  return LinearConstraint<
      Number,
      VariableRef >(std::move(e) - n,
                    LinearConstraint< Number, VariableRef >::Disequation);
}

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator!=(
    LinearExpression< Number, VariableRef > x,
    VariableExpression< Number, VariableRef > y) {
  return LinearConstraint<
      Number,
      VariableRef >(std::move(x) - std::move(y),
                    LinearConstraint< Number, VariableRef >::Disequation);
}

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator!=(
    VariableExpression< Number, VariableRef > x, const Number& n) {
  return LinearConstraint<
      Number,
      VariableRef >(std::move(x) - n,
                    LinearConstraint< Number, VariableRef >::Disequation);
}

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator!=(
    VariableExpression< Number, VariableRef > x, int n) {
  return LinearConstraint<
      Number,
      VariableRef >(std::move(x) - n,
                    LinearConstraint< Number, VariableRef >::Disequation);
}

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator!=(
    VariableExpression< Number, VariableRef > x,
    VariableExpression< Number, VariableRef > y) {
  return LinearConstraint<
      Number,
      VariableRef >(std::move(x) - std::move(y),
                    LinearConstraint< Number, VariableRef >::Disequation);
}

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator!=(
    VariableExpression< Number, VariableRef > x,
    LinearExpression< Number, VariableRef > y) {
  return LinearConstraint<
      Number,
      VariableRef >(std::move(y) - std::move(x),
                    LinearConstraint< Number, VariableRef >::Disequation);
}

template < typename Number, typename VariableRef >
inline LinearConstraint< Number, VariableRef > operator!=(
    LinearExpression< Number, VariableRef > x,
    const LinearExpression< Number, VariableRef >& y) {
  return LinearConstraint<
      Number,
      VariableRef >(std::move(x) - y,
                    LinearConstraint< Number, VariableRef >::Disequation);
}

/// @}

/// \brief Represents a set of linear constraints
template < typename Number, typename VariableRef >
class LinearConstraintSystem {
public:
  using LinearConstraintT = LinearConstraint< Number, VariableRef >;

private:
  using Constraints = std::vector< LinearConstraintT >;

public:
  using Iterator = typename Constraints::iterator;
  using ConstIterator = typename Constraints::const_iterator;

private:
  Constraints _csts;

public:
  /// \brief Create an empty system of constraints
  LinearConstraintSystem() = default;

  /// \brief Create a system of constraints with the given constraint
  explicit LinearConstraintSystem(LinearConstraintT cst)
      : _csts{std::move(cst)} {}

  /// \brief Create a system of constraints with the given constraints
  LinearConstraintSystem(std::initializer_list< LinearConstraintT > csts)
      : _csts(std::move(csts)) {}

  /// \brief Copy constructor
  LinearConstraintSystem(const LinearConstraintSystem&) = default;

  /// \brief Move constructor
  LinearConstraintSystem(LinearConstraintSystem&&) = default;

  /// \brief Copy assignment operator
  LinearConstraintSystem& operator=(const LinearConstraintSystem&) = default;

  /// \brief Move assignment operator
  LinearConstraintSystem& operator=(LinearConstraintSystem&&) = default;

  /// \brief Destructor
  ~LinearConstraintSystem() = default;

  /// \brief Return true if the system is empty
  bool empty() const { return this->_csts.empty(); }

  /// \brief Return the number of constraints
  std::size_t size() const { return this->_csts.size(); }

  /// \brief Add a constraint
  void add(LinearConstraintT cst) { this->_csts.emplace_back(std::move(cst)); }

  /// \brief Add a system of constraints
  void add(const LinearConstraintSystem& csts) {
    this->_csts.reserve(this->_csts.size() + csts.size());
    this->_csts.insert(this->_csts.end(), csts.begin(), csts.end());
  }

  /// \brief Add a system of constraints
  void add(LinearConstraintSystem&& csts) {
    this->_csts.reserve(this->_csts.size() + csts.size());
    this->_csts.insert(this->_csts.end(),
                       std::make_move_iterator(csts.begin()),
                       std::make_move_iterator(csts.end()));
  }

  /// \brief Return the begin iterator over the constraints
  Iterator begin() { return this->_csts.begin(); }
  ConstIterator begin() const { return this->_csts.begin(); }
  ConstIterator cbegin() const { return this->_csts.cbegin(); }

  /// \brief Return the end iterator over the terms
  Iterator end() { return this->_csts.end(); }
  ConstIterator end() const { return this->_csts.end(); }
  ConstIterator cend() const { return this->_csts.cend(); }

  /// \brief Return the set of variables present in the system
  PatriciaTreeSet< VariableRef > variables() const {
    PatriciaTreeSet< VariableRef > vars;
    for (const LinearConstraintT& cst : this->_csts) {
      for (const auto& term : cst) {
        vars.insert(term.first);
      }
    }
    return vars;
  }

  /// \brief Dump the linear constraint system, for debugging purpose
  void dump(std::ostream& o) const {
    o << "{";
    for (auto it = this->_csts.begin(), et = this->_csts.end(); it != et;) {
      it->dump(o);
      ++it;
      if (it != et) {
        o << "; ";
      }
    }
    o << "}";
  }

}; // end class LinearConstraintSystem

/// \brief Write a linear constraint system on a stream
template < typename Number, typename VariableRef >
inline std::ostream& operator<<(
    std::ostream& o, const LinearConstraintSystem< Number, VariableRef >& e) {
  e.dump(o);
  return o;
}

} // end namespace core
} // end namespace ikos
