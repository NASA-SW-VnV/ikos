/*******************************************************************************
 *
 * \file
 * \brief Data structure for the symbolic manipulation of linear expressions
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2018-2019 United States Government as represented by the
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

#include <iostream>

#include <boost/container/flat_map.hpp>
#include <boost/optional.hpp>

#include <ikos/core/adt/patricia_tree/set.hpp>
#include <ikos/core/semantic/dumpable.hpp>
#include <ikos/core/semantic/indexable.hpp>
#include <ikos/core/semantic/variable.hpp>

namespace ikos {
namespace core {

/// \brief Represents a typed variable
///
/// This is just a thin wrapper on VariableRef.
template < typename Number, typename VariableRef >
class VariableExpression {
public:
  static_assert(
      IsVariable< VariableRef >::value,
      "VariableRef does not meet the requirements for variable types");

private:
  VariableRef _var;

public:
  /// \brief No default constructor
  VariableExpression() = delete;

  /// \brief Constructor
  explicit VariableExpression(VariableRef var) : _var(std::move(var)) {}

  /// \brief Copy constructor
  VariableExpression(const VariableExpression&) noexcept = default;

  /// \brief Move constructor
  VariableExpression(VariableExpression&&) noexcept = default;

  /// \brief Copy assignment operator
  VariableExpression& operator=(const VariableExpression&) noexcept = default;

  /// \brief Move assignment operator
  VariableExpression& operator=(VariableExpression&&) noexcept = default;

  /// \brief Destructor
  ~VariableExpression() = default;

  /// \brief Return the variable reference
  VariableRef var() const { return this->_var; }

  /// \brief Dump the variable, for debugging purpose
  void dump(std::ostream& o) const {
    DumpableTraits< VariableRef >::dump(o, this->_var);
  }

}; // end class VariableExpression

/// \brief Represents a linear expression
template < typename Number, typename VariableRef >
class LinearExpression {
public:
  static_assert(
      IsVariable< VariableRef >::value,
      "VariableRef does not meet the requirements for variable types");

public:
  using VariableExpressionT = VariableExpression< Number, VariableRef >;

private:
  using Map = boost::container::flat_map< VariableRef, Number >;

public:
  /// \brief Iterator over the terms
  using TermIterator = typename Map::iterator;

  /// \brief Constant iterator over the terms
  using TermConstIterator = typename Map::const_iterator;

private:
  Map _map;
  Number _cst;

public:
  /// \brief Create a linear expression equals to zero
  LinearExpression() = default;

  /// \brief Create a constant expression
  explicit LinearExpression(Number n) : _cst(std::move(n)) {}

  /// \brief Create a constant expression
  explicit LinearExpression(int n) : _cst(n) {}

  /// \brief Create a variable expression
  explicit LinearExpression(VariableRef var) {
    this->_map.emplace(var, Number(1));
  }

  /// \brief Create a variable expression
  explicit LinearExpression(VariableExpressionT e) {
    this->_map.emplace(e.var(), Number(1));
  }

  /// \brief Create the expression cst * var
  LinearExpression(Number cst, VariableRef var) {
    if (cst != 0) {
      this->_map.emplace(var, std::move(cst));
    }
  }

  /// \brief Create the expression cst * var
  LinearExpression(int cst, VariableRef var) {
    if (cst != 0) {
      this->_map.emplace(var, Number(cst));
    }
  }

  /// \brief Copy constructor
  LinearExpression(const LinearExpression&) = default;

  /// \brief Move constructor
  LinearExpression(LinearExpression&&) = default;

  /// \brief Copy assignment operator
  LinearExpression& operator=(const LinearExpression&) = default;

  /// \brief Move assignment operator
  LinearExpression& operator=(LinearExpression&&) = default;

  /// \brief Destructor
  ~LinearExpression() = default;

private:
  /// \brief Private constructor
  LinearExpression(Map map, Number cst)
      : _map(std::move(map)), _cst(std::move(cst)) {}

public:
  /// \brief Add a constant
  void add(const Number& n) { this->_cst += n; }

  /// \brief Add a constant
  void add(int n) { this->_cst += n; }

  /// \brief Add a variable
  void add(VariableRef var) { this->add(1, var); }

  /// \brief Add a term cst * var
  void add(const Number& cst, VariableRef var) {
    auto it = this->_map.find(var);
    if (it != this->_map.end()) {
      Number r = it->second + cst;
      if (r == 0) {
        this->_map.erase(it);
      } else {
        it->second = r;
      }
    } else {
      if (cst != 0) {
        this->_map.emplace(var, cst);
      }
    }
  }

  /// \brief Add a term cst * var
  void add(int cst, VariableRef var) {
    auto it = this->_map.find(var);
    if (it != this->_map.end()) {
      Number r = it->second + cst;
      if (r == 0) {
        this->_map.erase(it);
      } else {
        it->second = r;
      }
    } else {
      if (cst != 0) {
        this->_map.emplace(var, Number(cst));
      }
    }
  }

  /// \brief Return the begin iterator over the terms
  TermIterator begin() { return this->_map.begin(); }
  TermConstIterator begin() const { return this->_map.begin(); }
  TermConstIterator cbegin() const { return this->_map.cbegin(); }

  /// \brief Return the end iterator over the terms
  TermIterator end() { return this->_map.end(); }
  TermConstIterator end() const { return this->_map.end(); }
  TermConstIterator cend() const { return this->_map.cend(); }

  /// \brief Return the number of terms
  std::size_t num_terms() const { return this->_map.size(); }

  /// \brief Return true if the linear expression is constant
  bool is_constant() const { return this->_map.empty(); }

  /// \brief Return the constant
  const Number& constant() const { return this->_cst; }

  /// \brief Return the factor for the given variable
  Number factor(VariableRef var) const {
    auto it = this->_map.find(var);
    if (it != this->_map.end()) {
      return it->second;
    } else {
      return Number(0);
    }
  }

  /// \brief Add a number
  void operator+=(const Number& n) { this->_cst += n; }

  /// \brief Add a number
  void operator+=(int n) { this->_cst += n; }

  /// \brief Add a variable
  void operator+=(VariableRef var) { this->add(var); }

  /// \brief Add a linear expression
  void operator+=(const LinearExpression& expr) {
    for (const auto& term : expr) {
      this->add(term.second, term.first);
    }
    this->_cst += expr.constant();
  }

  /// \brief Substract a number
  void operator-=(const Number& n) { this->_cst -= n; }

  /// \brief Substract a number
  void operator-=(int n) { this->_cst -= n; }

  /// \brief Substract a variable
  void operator-=(VariableRef var) { this->add(-1, var); }

  /// \brief Substract a linear expression
  void operator-=(const LinearExpression& expr) {
    for (const auto& term : expr) {
      this->add(-term.second, term.first);
    }
    this->_cst -= expr.constant();
  }

  /// \brief Unary minus
  LinearExpression operator-() const {
    LinearExpression r(*this);
    r *= -1;
    return r;
  }

  /// \brief Multiply by a constant
  void operator*=(const Number& n) {
    if (n == 0) {
      this->_map.clear();
      this->_cst = 0;
    } else {
      for (auto& term : this->_map) {
        term.second *= n;
      }
      this->_cst *= n;
    }
  }

  /// \brief Multiply by a constant
  void operator*=(int n) {
    if (n == 0) {
      this->_map.clear();
      this->_cst = 0;
    } else {
      for (auto& term : this->_map) {
        term.second *= n;
      }
      this->_cst *= n;
    }
  }

  /// \brief If the linear expression is just a variable v, return v, otherwise
  /// return boost::none.
  boost::optional< VariableRef > variable() const {
    if (this->_cst == 0 && this->_map.size() == 1) {
      auto it = this->_map.begin();
      if (it->second == 1) {
        return it->first;
      }
    }
    return boost::none;
  }

  /// \brief Return the set of variables present in the linear expression
  PatriciaTreeSet< VariableRef > variables() const {
    PatriciaTreeSet< VariableRef > vars;
    for (const auto& term : this->_map) {
      vars.insert(term.first);
    }
    return vars;
  }

  /// \brief Dump the linear expression, for debugging purpose
  void dump(std::ostream& o) const {
    for (auto it = this->_map.begin(), et = this->_map.end(); it != et; ++it) {
      const Number& cst = it->second;
      VariableRef var = it->first;
      if (cst > 0 && it != this->_map.begin()) {
        o << "+";
      }
      if (cst == -1) {
        o << "-";
      } else if (cst != 1) {
        o << cst;
      }
      DumpableTraits< VariableRef >::dump(o, var);
    }
    if (this->_cst > 0 && !this->_map.empty()) {
      o << "+";
    }
    if (this->_cst != 0 || this->_map.empty()) {
      o << this->_cst;
    }
  }

}; // end class LinearExpression

/// \brief Write a linear expression on a stream
template < typename Number, typename VariableRef >
inline std::ostream& operator<<(
    std::ostream& o, const LinearExpression< Number, VariableRef >& e) {
  e.dump(o);
  return o;
}

/// \name Operator *
/// @{

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator*(
    VariableExpression< Number, VariableRef > e, Number n) {
  return LinearExpression< Number, VariableRef >(std::move(n), e.var());
}

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator*(
    VariableExpression< Number, VariableRef > e, int n) {
  return LinearExpression< Number, VariableRef >(n, e.var());
}

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator*(
    Number n, VariableExpression< Number, VariableRef > e) {
  return LinearExpression< Number, VariableRef >(std::move(n), e.var());
}

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator*(
    int n, VariableExpression< Number, VariableRef > e) {
  return LinearExpression< Number, VariableRef >(n, e.var());
}

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator*(
    LinearExpression< Number, VariableRef > e, const Number& n) {
  e *= n;
  return e;
}

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator*(
    LinearExpression< Number, VariableRef > e, int n) {
  e *= n;
  return e;
}

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator*(
    const Number& n, LinearExpression< Number, VariableRef > e) {
  e *= n;
  return e;
}

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator*(
    int n, LinearExpression< Number, VariableRef > e) {
  e *= n;
  return e;
}

/// @}
/// \name Operator +
/// @{

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator+(
    VariableExpression< Number, VariableRef > e, const Number& n) {
  LinearExpression< Number, VariableRef > r(e.var());
  r += n;
  return r;
}

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator+(
    VariableExpression< Number, VariableRef > e, int n) {
  LinearExpression< Number, VariableRef > r(e.var());
  r += n;
  return r;
}

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator+(
    const Number& n, VariableExpression< Number, VariableRef > e) {
  LinearExpression< Number, VariableRef > r(e.var());
  r += n;
  return r;
}

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator+(
    int n, VariableExpression< Number, VariableRef > e) {
  LinearExpression< Number, VariableRef > r(e.var());
  r += n;
  return r;
}

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator+(
    VariableExpression< Number, VariableRef > x,
    VariableExpression< Number, VariableRef > y) {
  LinearExpression< Number, VariableRef > r(x.var());
  r += y.var();
  return r;
}

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator+(
    LinearExpression< Number, VariableRef > e, const Number& n) {
  e += n;
  return e;
}

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator+(
    LinearExpression< Number, VariableRef > e, int n) {
  e += n;
  return e;
}

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator+(
    const Number& n, LinearExpression< Number, VariableRef > e) {
  e += n;
  return e;
}

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator+(
    int n, LinearExpression< Number, VariableRef > e) {
  e += n;
  return e;
}

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator+(
    LinearExpression< Number, VariableRef > x,
    VariableExpression< Number, VariableRef > y) {
  x += y.var();
  return x;
}

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator+(
    VariableExpression< Number, VariableRef > x,
    LinearExpression< Number, VariableRef > y) {
  y += x.var();
  return y;
}

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator+(
    LinearExpression< Number, VariableRef > x,
    const LinearExpression< Number, VariableRef >& y) {
  x += y;
  return x;
}

/// @}
/// \name Operator -
/// @{

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator-(
    VariableExpression< Number, VariableRef > e, const Number& n) {
  LinearExpression< Number, VariableRef > r(e.var());
  r -= n;
  return r;
}

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator-(
    VariableExpression< Number, VariableRef > e, int n) {
  LinearExpression< Number, VariableRef > r(e.var());
  r -= n;
  return r;
}

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator-(
    const Number& n, VariableExpression< Number, VariableRef > e) {
  LinearExpression< Number, VariableRef > r(-1, e.var());
  r += n;
  return r;
}

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator-(
    int n, VariableExpression< Number, VariableRef > e) {
  LinearExpression< Number, VariableRef > r(-1, e.var());
  r += n;
  return r;
}

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator-(
    VariableExpression< Number, VariableRef > x,
    VariableExpression< Number, VariableRef > y) {
  LinearExpression< Number, VariableRef > r(x.var());
  r -= y.var();
  return r;
}

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator-(
    LinearExpression< Number, VariableRef > e, const Number& n) {
  e -= n;
  return e;
}

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator-(
    LinearExpression< Number, VariableRef > e, int n) {
  e -= n;
  return e;
}

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator-(
    LinearExpression< Number, VariableRef > x,
    VariableExpression< Number, VariableRef > y) {
  x -= y.var();
  return x;
}

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator-(
    const Number& n, LinearExpression< Number, VariableRef > e) {
  e *= -1;
  e += n;
  return e;
}

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator-(
    int n, LinearExpression< Number, VariableRef > e) {
  e *= -1;
  e += n;
  return e;
}

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator-(
    VariableExpression< Number, VariableRef > x,
    LinearExpression< Number, VariableRef > y) {
  y *= -1;
  y += x.var();
  return y;
}

template < typename Number, typename VariableRef >
inline LinearExpression< Number, VariableRef > operator-(
    LinearExpression< Number, VariableRef > x,
    const LinearExpression< Number, VariableRef >& y) {
  x -= y;
  return x;
}

/// @}

} // end namespace core
} // end namespace ikos
