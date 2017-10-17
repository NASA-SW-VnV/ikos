/**************************************************************************/ /**
 *
 * \file
 * \brief Value for constant domain.
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

#ifndef IKOS_VALUE_CONSTANT_HPP
#define IKOS_VALUE_CONSTANT_HPP

#include <boost/optional.hpp>

#include <ikos/common/types.hpp>
#include <ikos/number.hpp>

namespace ikos {

/// \brief Constant abstract value
///
/// This is either top, bottom or a constant.
template < typename Number >
class constant : public writeable {
public:
  typedef constant< Number > constant_t;

private:
  typedef enum { BOTTOM, TOP, NUMBER } kind_t;

private:
  kind_t _kind;
  Number _n;

private:
  constant();

  constant(bool bottom) : _kind(bottom ? BOTTOM : TOP), _n(0) {}

  constant(kind_t kind, Number n) : _kind(kind), _n(n) {}

public:
  static constant_t bottom() { return constant_t(true); }

  static constant_t top() { return constant_t(false); }

public:
  constant(Number n) : _kind(NUMBER), _n(n) {}

  template < typename N,
             class = std::enable_if_t< is_supported_integral< N >::value > >
  constant(N n) : _kind(NUMBER), _n(n) {}

  constant(const constant_t& other) : _kind(other._kind), _n(other._n) {}

  constant_t& operator=(constant_t other) {
    this->_kind = other._kind;
    this->_n = other._n;
    return *this;
  }

  bool is_bottom() const { return this->_kind == BOTTOM; }

  bool is_top() const { return this->_kind == TOP; }

  bool is_number() const { return this->_kind == NUMBER; }

  boost::optional< Number > number() const {
    if (this->_kind == NUMBER) {
      return boost::optional< Number >(this->_n);
    } else {
      return boost::optional< Number >();
    }
  }

  constant_t operator-() const { return constant_t(this->_kind, -this->_n); }

  constant_t operator+(constant_t other) const {
    if (this->is_bottom() || other.is_bottom()) {
      return bottom();
    } else if (this->is_top() || other.is_top()) {
      return top();
    } else {
      return constant_t(this->_n + other._n);
    }
  }

  constant_t operator-(constant_t other) const {
    if (this->is_bottom() || other.is_bottom()) {
      return bottom();
    } else if (this->is_top() || other.is_top()) {
      return top();
    } else {
      return constant_t(this->_n - other._n);
    }
  }

  constant_t operator*(constant_t other) const {
    if (this->is_bottom() || other.is_bottom()) {
      return bottom();
    } else if (*this == constant(0) || other == constant(0)) {
      return constant_t(0);
    } else if (this->is_top() || other.is_top()) {
      return top();
    } else {
      return constant_t(this->_n * other._n);
    }
  }

  constant_t operator/(constant_t other) const {
    if (this->is_bottom() || other.is_bottom()) {
      return bottom();
    } else if (this->is_top() || other.is_top()) {
      return top();
    } else if (other._n == 0) {
      return bottom();
    } else {
      return constant_t(this->_n / other._n);
    }
  }

  constant_t& operator+=(constant_t other) {
    return this->operator=(this->operator+(other));
  }

  constant_t& operator-=(constant_t other) {
    return this->operator=(this->operator-(other));
  }

  constant_t& operator*=(constant_t other) {
    return this->operator=(this->operator*(other));
  }

  constant_t& operator/=(constant_t other) {
    return this->operator=(this->operator/(other));
  }

  bool operator<=(constant_t other) const {
    if (this->_kind == BOTTOM) {
      return true;
    } else if (this->_kind == TOP) {
      return other._kind == TOP;
    } else if (this->_kind == NUMBER) {
      return (other._kind == NUMBER) && (this->_n == other._n);
    } else {
      return false;
    }
  }

  bool operator==(constant_t other) const {
    if (this->_kind == NUMBER) {
      return (other._kind == NUMBER) && (this->_n == other._n);
    } else {
      return this->_kind == other._kind;
    }
  }

  bool operator!=(constant_t other) const { return !(this->operator==(other)); }

  constant_t operator|(constant_t other) const {
    if (this->is_bottom()) {
      return other;
    } else if (other.is_bottom()) {
      return *this;
    } else if (this->_kind == NUMBER && other._kind == NUMBER &&
               this->_n == other._n) {
      return *this;
    } else {
      return top();
    }
  }

  constant_t operator||(constant_t other) const {
    return this->operator|(other);
  }

  constant_t operator&(constant_t other) const {
    if (this->is_bottom() || other.is_bottom()) {
      return bottom();
    } else if (this->_kind == NUMBER && other._kind == NUMBER &&
               this->_n == other._n) {
      return *this;
    } else {
      return bottom();
    }
  }

  constant_t operator&&(constant_t other) const {
    return this->operator&(other);
  }

  // division and remainder operations

  constant_t UDiv(constant_t x) const {
    if (this->is_bottom() || x.is_bottom()) {
      return constant_t::bottom();
    } else {
      return constant_t::top();
    }
  }

  constant_t SRem(constant_t x) const {
    if (this->is_bottom() || x.is_bottom()) {
      return constant_t::bottom();
    } else if (this->is_top() || x.is_top()) {
      return constant_t::top();
    } else if (x._n == 0) {
      return bottom();
    } else {
      return constant_t(this->_n % x._n);
    }
  }

  constant_t URem(constant_t x) const {
    if (this->is_bottom() || x.is_bottom()) {
      return constant_t::bottom();
    } else if (this->is_top() || x.is_top()) {
      return constant_t::top();
    } else if (x._n == 0) {
      return bottom();
    } else if (this->_n >= 0 && x._n > 0) {
      return constant_t(this->_n % x._n);
    } else {
      return constant_t::top();
    }
  }

  // bitwise operations

  constant_t Trunc(uint64_t from, uint64_t to) const {
    if (this->is_bottom()) {
      return constant_t::bottom();
    } else if (this->is_top()) {
      return constant_t::top();
    } else {
      return constant_t(this->_n & ((Number(1) << to) - 1));
    }
  }

  constant_t ZExt(uint64_t from, uint64_t to) const {
    if (this->is_bottom()) {
      return constant_t::bottom();
    } else if (this->is_top()) {
      return constant_t::top();
    } else {
      Number unsigned_max = (Number(1) << from) - 1;
      Number signed_min = -(Number(1) << (from - 1));
      if (this->_n >= 0 && this->_n <= unsigned_max) { // positive
        return *this;
      } else if (this->_n >= signed_min && this->_n < 0) { // negative
        return constant_t(this->_n % (Number(1) << from));
      } else {
        return constant_t::top();
      }
    }
  }

  constant_t SExt(uint64_t from, uint64_t to) const {
    if (this->is_bottom()) {
      return constant_t::bottom();
    } else if (this->is_top()) {
      return constant_t::top();
    } else {
      Number unsigned_max = (Number(1) << from) - 1;
      Number signed_min = -(Number(1) << (from - 1));
      if (this->_n >= signed_min && this->_n <= unsigned_max) {
        return *this;
      } else {
        return constant_t::top();
      }
    }
  }

  constant_t And(constant_t x) const {
    if (this->is_bottom() || x.is_bottom()) {
      return constant_t::bottom();
    } else if (this->is_top() || x.is_top()) {
      return constant_t::top();
    }
    return constant_t(this->_n & x._n);
  }

  constant_t Or(constant_t x) const {
    if (this->is_bottom() || x.is_bottom()) {
      return constant_t::bottom();
    } else if (this->is_top() || x.is_top()) {
      return constant_t::top();
    }
    return constant_t(this->_n | x._n);
  }

  constant_t Xor(constant_t x) const {
    if (this->is_bottom() || x.is_bottom()) {
      return constant_t::bottom();
    } else if (this->is_top() || x.is_top()) {
      return constant_t::top();
    }
    return constant_t(this->_n ^ x._n);
  }

  constant_t Shl(constant_t x) const {
    if (this->is_bottom() || x.is_bottom()) {
      return constant_t::bottom();
    } else if (this->is_top() || x.is_top()) {
      return constant_t::top();
    }
    return constant_t(this->_n << x._n);
  }

  constant_t LShr(constant_t x) const {
    if (this->is_bottom() || x.is_bottom()) {
      return constant_t::bottom();
    } else {
      return constant_t::top();
    }
  }

  constant_t AShr(constant_t x) const {
    if (this->is_bottom() || x.is_bottom()) {
      return constant_t::bottom();
    } else if (this->is_top() || x.is_top()) {
      return constant_t::top();
    } else {
      return constant_t(this->_n >> x._n);
    }
  }

  void write(std::ostream& o) {
    switch (this->_kind) {
      case BOTTOM: {
        o << "_|_";
        break;
      }
      case TOP: {
        o << "T";
        break;
      }
      case NUMBER: {
        o << this->_n;
        break;
      }
    }
  }

}; // end class constant

template < typename Number >
inline constant< Number > operator+(Number c, constant< Number > x) {
  return constant< Number >(c) + x;
}

template < typename Number >
inline constant< Number > operator+(constant< Number > x, Number c) {
  return x + constant< Number >(c);
}

template < typename Number >
inline constant< Number > operator*(Number c, constant< Number > x) {
  return constant< Number >(c) * x;
}

template < typename Number >
inline constant< Number > operator*(constant< Number > x, Number c) {
  return x * constant< Number >(c);
}

template < typename Number >
inline constant< Number > operator/(Number c, constant< Number > x) {
  return constant< Number >(c) / x;
}

template < typename Number >
inline constant< Number > operator/(constant< Number > x, Number c) {
  return x / constant< Number >(c);
}

template < typename Number >
inline constant< Number > operator-(Number c, constant< Number > x) {
  return constant< Number >(c) - x;
}

template < typename Number >
inline constant< Number > operator-(constant< Number > x, Number c) {
  return x - constant< Number >(c);
}

typedef constant< z_number > z_constant;
typedef constant< q_number > q_constant;

} // end namespace ikos

#endif // IKOS_VALUE_CONSTANT_HPP
