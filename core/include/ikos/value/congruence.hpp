/**************************************************************************/ /**
 *
 * \file
 * \brief Values for congruence domain operations.
 *
 * Author: Alexandre C. D. Wimmers
 *
 * Contributors: Jorge A. Navas
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

#ifndef IKOS_VALUE_CONGRUENCE_HPP
#define IKOS_VALUE_CONGRUENCE_HPP

#include <iosfwd>

#include <ikos/common/types.hpp>
#include <ikos/number/support.hpp>

namespace ikos {

/// \brief Congruence abstract value.
///
/// This is either bottom, or aZ + b.
///
/// Note that top is represented as 1Z + 0.
///
/// TypeSize = -1 means unlimited precision.
template < typename Number, int TypeSize = -1 >
class congruence : public writeable {
public:
  typedef congruence< Number, TypeSize > congruence_t;

private:
  bool _is_bottom;
  Number _a, _b; // of the form (aZ + b)

  static bool is_unlimited(int width) { return width == -1; }

  congruence(bool b) : _is_bottom(!b), _a(1), _b(0) {}

  static Number abs(Number x) { return x < 0 ? -x : x; }

  static Number max(Number x, Number y) { return x <= y ? y : x; }

  static Number min(Number x, Number y) { return x < y ? x : y; }

  static Number gcd(Number x, Number y, Number z) { return gcd(x, gcd(y, z)); }

  static Number gcd(Number x, Number y) { return gcd_helper(abs(x), abs(y)); }

  // Not to be called explicitly outside of _gcd
  static Number gcd_helper(Number x, Number y) {
    return (y == 0) ? x : gcd_helper(y, x % y);
  }

  static Number lcm(Number x, Number y) {
    Number tmp = gcd(x, y);
    if (tmp == 0)
      throw logic_error("congruence: lcm causes division by zero");
    return abs(x * y) / tmp;
  }

  /// \brief Weakening operator
  ///
  /// \returns weakened version of _this, does not modify
  /// _this. Returned _a is guaranteed to be a power of 2.
  congruence_t weakened() const {
    ikos_assert(TypeSize > -1);

    if (_a == 0) {
      return *this;
    }

    if (TypeSize < 32) {
      Number tmp = gcd(_a, Number(1) << TypeSize);
      if (tmp == 0)
        throw logic_error("congruence: weakened causes division by zero");
      return congruence_t(tmp, _b % tmp);
    } else {
      throw logic_error(
          "congruence: if int is 32 bits shift can only range from 0 to 31");
    }
  }

  /// \brief Searches for the right-most bit containing a value of 1.
  Number scanr1(Number x) const {
    ikos_assert(TypeSize > -1);

    Number count(1);
    for (Number idx(1); idx < TypeSize + 1;) {
      if ((idx & x) > 0) {
        return count;
      }
      ++count;
      idx = idx << 1;
    }
    return Number(-1); // to indicate not found, x==0
  }

  /// \brief Searches for the left-most bit containing a value of 1.
  Number scanl1(Number x) const {
    ikos_assert(TypeSize > -1);

    if (TypeSize < 32) {
      Number count(sizeof(x));
      Number idx = Number(1) << (TypeSize - 1);
      while (idx > 0) {
        if ((idx & x) > 0) {
          return count;
        }
        --count;
        idx = idx >> 1;
      }
      return Number(-1); // to indicate not found, x==0
    } else {
      throw logic_error(
          "congruence: if int is 32 bits shift can only range from 0 to 31");
    }
  }

public:
  static congruence_t top() { return congruence(true); }

  static congruence_t bottom() { return congruence(false); }

  congruence() : _is_bottom(false), _a(1), _b(0) {}

  congruence(Number b) : _is_bottom(false), _a(0), _b(b) {}

  template < typename N,
             class = std::enable_if_t< is_supported_integral< N >::value > >
  congruence(N n) : _is_bottom(false), _a(0), _b(n) {}

  congruence(Number a, Number b)
      : _is_bottom(false),
        _a(a),
        // set to standard form: 0 <= b < a for a != 0.
        _b((a == 0) ? b : ((b % a) < 0 ? (b % a) + a : b % a)) {}

  congruence(const congruence_t& o)
      : _is_bottom(o._is_bottom), _a(o._a), _b(o._b) {}

  congruence_t operator=(congruence_t o) {
    _is_bottom = o._is_bottom;
    _a = o._a;
    _b = o._b;
    return *this;
  }

  bool is_bottom() const { return _is_bottom; }

  bool is_top() const { return _a == 1; }

  std::pair< Number, Number > get() const {
    // aZ+b
    return std::make_pair(_a, _b);
  }

  boost::optional< Number > singleton() const {
    if (!this->is_bottom() && _a == 0) {
      return boost::optional< Number >(_b);
    } else {
      return boost::optional< Number >();
    }
  }

  bool operator==(congruence_t o) const {
    return _a == o._a && _b == o._b && is_bottom() == o.is_bottom();
  }

  bool operator!=(congruence_t x) const { return !this->operator==(x); }

  // Lattice Operations

  bool operator<=(const congruence_t o) const {
    if (is_bottom()) {
      return true;
    } else if (o.is_bottom()) {
      return false;
    } else if (_a == 0 && o._a == 0) {
      return _b == o._b;
    } else if (_a == 0) {
      return (_b % o._a < 0 ? (_b % o._a) + o._a : _b % o._a) == o._b % o._a;
    } else if (o._a == 0) {
      return false;
    } else {
      return (_a % o._a == 0) && (_b % o._a == o._b % o._a);
    }
  }

  congruence_t operator|(congruence_t o) const {
    if (is_bottom()) {
      return o;
    } else if (o.is_bottom()) {
      return *this;
    }
    return congruence_t(gcd(_a, o._a, abs(_b - o._b)), min(_b, o._b));
  }

  congruence_t operator||(congruence_t o) const {
    // equivalent to join, domain is flat
    return *this | o;
  }

  congruence_t operator&(congruence_t o) const {
    if (is_bottom() || o.is_bottom()) {
      return bottom();
    }

    // note: lcm has meaning only if both a and o.a are not 0
    if (_a == 0 && o._a == 0) {
      if (_b == o._b) {
        return *this;
      } else {
        return bottom();
      }
    } else if (_a == 0) {
      // b & a'Z + b' iff \exists k such that a'*k + b' = b iff ((b - b') %a' ==
      // 0)
      if ((_b - o._b) % o._a == 0) {
        return *this;
      } else {
        return bottom();
      }
    } else if (o._a == 0) {
      // aZ+b & b' iff \exists k such that a*k+b  = b' iff ((b'-b %a) == 0)
      if ((o._b - _b) % _a == 0) {
        return o;
      } else {
        return bottom();
      }
    } else {
      // pre: a and o.a != 0
      Number x = gcd(_a, o._a);
      if (x == 0)
        throw logic_error("congruence: operator& causes division by zero");
      if (_b % x == (o._b % x)) {
        // the part max(b,o.b) needs to be verified. What we really
        // want is to find b'' such that
        // 1) b'' % lcm(a,a') == b  % lcm(a,a'), and
        // 2) b'' % lcm(a,a') == b' % lcm(a,a').
        // An algorithm for that is provided in Granger'89.
        return congruence_t(lcm(_a, o._a), max(_b, o._b));
      } else {
        return congruence_t::bottom();
      }
    }

    ///
    // The below code is harder to verify. E.g. 2Z + 0 & 1 is
    // bottom, however, by the 4th rule it will return 1.
    ///
    // if (_a == 0 && o._a == 0 && _b == o._b) {
    //   return *this;
    // } else if(_a == 0 && o._a == 0) {
    //   return bottom();
    // } else if(_a == 0 && ((_b % o._a < 0 ? (_b % o._a) + o._a : _b % o._a) ==
    // o._b % o._a)) {
    //   return *this;
    // } else if(o._a == 0 && ((o._b % _a < 0 ? (o._b % _a) + _a : o._b % _a) ==
    // o._b % _a)) {
    //   return o;
    // } else if(_b % gcd(_a, o._a) == (o._b % gcd(_a, o._a))) {
    //   // This case needs to be verified.
    //   return congruence_t(lcm(_a, o._a), max(_b, o._b));
    // }
    // return bottom();
  }

  congruence_t operator&&(congruence_t o) const {
    // simply refines top element
    return (is_top()) ? o : *this;
  }

  // Arithmetic Operators

  congruence_t operator+(congruence_t o) const {
    if (this->is_bottom() || o.is_bottom())
      return congruence_t::bottom();
    else if (this->is_top() || o.is_top())
      return congruence_t::top();
    else
      return congruence_t(gcd(_a, o._a), _b + o._b);
  }

  congruence_t operator-(congruence_t o) const {
    if (this->is_bottom() || o.is_bottom())
      return congruence_t::bottom();
    else if (this->is_top() || o.is_top())
      return congruence_t::top();
    else
      return congruence_t(gcd(_a, o._a), _b - o._b);
  }

  congruence_t operator-() const {
    if (this->is_bottom() || this->is_top())
      return *this;
    else
      return congruence_t(_a, -_b + _a);
  }

  congruence_t operator*(congruence_t o) const {
    if (this->is_bottom() || o.is_bottom())
      return congruence_t::bottom();
    else if ((this->is_top() || o.is_top()) && _a != 0 && o._a != 0)
      return congruence_t::top();
    else
      return congruence_t(gcd(_a * o._a, _a * o._b, o._a * _b), _b * o._b);
  }

  /// \brief Signed division
  congruence_t operator/(congruence_t o) const {
    if (this->is_bottom() || o.is_bottom())
      return congruence_t::bottom();
    else if (o == congruence(0))
      return congruence_t::bottom();
    else if (this->is_top() || o.is_top())
      return congruence_t::top();
    else {
      /*
         (Special case 1) aZ+b / 0Z+b':
         if b'|a then  (a/b')Z + b/b'
         else          top

         (Special case 2) 0Z+b / a'Z+b':
         if N>0   (b div N)Z + 0
         else     0Z + 0

         where N = a'((b-b') div a') + b'
      */

      if (o._a == 0) {
        if (o._b == 0)
          throw logic_error("congruence: operator/ causes division by zero");
        if (_a % o._b == 0)
          return congruence_t(_a / o._b, _b / o._b);
        else
          return congruence_t::top();
      }

      if (_a == 0) {
        Number n(o._a * (((_b - o._b) / o._a) + o._b));
        if (n > 0) {
          return congruence_t(_b / n, Number(0));
        } else {
          return congruence_t(Number(0), Number(0));
        }
      }

      /*
        General case: no singleton
      */
      return congruence_t::top();
    }
  }

  /// \brief Signed remainder operator
  congruence_t operator%(congruence_t o) const {
    if (this->is_bottom() || o.is_bottom())
      return congruence_t::bottom();
    else if (o == congruence(0))
      return congruence_t::bottom();
    else if (this->is_top() || o.is_top())
      return congruence_t::top();
    else {
      /*
       (Special case 1) aZ+b / 0Z+b':
         if b'|a then  0Z + (b mod b')
         else          gcd(a,b')Z + b

       (Special case 2) 0Z+b / a'Z+b':
         if N<=0           then 0Z+b
         if (b div N) == 1 then gcd(b',a')Z + b
         if (b div N) >= 2 then N(b div N)Z  + b

         where N = a'((b-b') div a') + b'
      */
      if (o._a == 0) {
        if (o._b == 0)
          throw logic_error("congruence: operator% causes division by zero");
        if (_a % o._b == 0) {
          return congruence_t(Number(0), _b % o._b);
        } else {
          return congruence_t(gcd(_a, o._b), _b);
        }
      }
      if (_a == 0) {
        Number n(o._a * (((_b - o._b) / o._a) + o._b));
        if (n <= 0) {
          return congruence_t(_a, _b);
        } else if (_b == n) {
          return congruence_t(gcd(o._b, o._a), _b);
        } else if ((_b / n) >= 2) {
          return congruence_t(_b, _b);
        } else {
          ikos_unreachable("unreachable");
        }
      }
      /*
          general case: no singleton
      */
      return congruence_t(gcd(_a, o._a, o._b), _b);
    }
  }

  // Bitwise operators

private:
  congruence_t Not() const {
    if (this->is_bottom())
      return congruence_t::bottom();
    else if (this->is_top())
      return congruence_t::top();
    else if (is_unlimited(TypeSize))
      return congruence_t::top();
    else {
      if (TypeSize < 32) {
        Number two_to_n = Number(1) << TypeSize;
        return congruence_t(gcd(_a, two_to_n), -_b - 1);
      } else {
        throw logic_error(
            "congruence: if int is 32 bits shift can only range from 0 to 31");
      }
    }
    // For simplicity, we don't implement the case when a==0:
    //   return (is_bottom()) ? bottom() :
    //   (a== 0) ? congruence_t(~b) : congruence_t(gcd(a, 1 << TypeSize), -b-
    //   1);
  }

public:
  congruence_t And(congruence_t o) const {
    if (this->is_bottom() || o.is_bottom())
      return congruence_t::bottom();
    else if (this->is_top() || o.is_top())
      return congruence_t::top();
    else if (is_unlimited(TypeSize))
      return congruence_t::top();
    else {
      // first both numbers are approximated to power of 2
      congruence_t x(this->weakened());
      congruence_t y(o.weakened());

      if (x._a == 0 && y._a == 0)
        return congruence_t(Number(0), x._b & y._b);

      if (x._a == 0) // (0Z + b) & (a'Z+b')
        return congruence_t((x._b < y._a) ? Number(0) : y._a, x._b & y._b);

      if (y._a == 0) // (aZ + b) & (0Z+b')
        return congruence_t((y._b < x._a) ? Number(0) : x._a, x._b & y._b);

      if (scanr1(y._b) > scanr1(y._a)) // choose the smallest power of 2
        return congruence_t(scanr1(y._a), x._b & y._b);
      else
        return congruence_t(scanr1(x._a), x._b & y._b);
    }
  }

  congruence_t Or(congruence_t o) const {
    if (this->is_bottom() || o.is_bottom())
      return congruence_t::bottom();
    else if (this->is_top() || o.is_top())
      return congruence_t::top();
    else if (is_unlimited(TypeSize))
      return congruence_t::top();
    else {
      return this->Not().And(o.Not()).Not();
    }
  }

  congruence_t Xor(congruence_t o) const {
    if (this->is_bottom() || o.is_bottom())
      return congruence_t::bottom();
    else if (this->is_top() || o.is_top())
      return congruence_t::top();
    else if (is_unlimited(TypeSize))
      return congruence_t::top();
    else {
      return this->And(o.Not()).Or(this->Not().And(o));
    }
  }

  congruence_t Shl(congruence_t o) const {
    if (this->is_bottom() || o.is_bottom())
      return congruence_t::bottom();
    else if (this->is_top() || o.is_top())
      return congruence_t::top();
    else if (is_unlimited(TypeSize))
      return congruence_t::top();
    else {
      if (o._a == 0) { // singleton
        // aZ + b << 0Z + b'  = (a*2^b')Z + b*2^b'
        Number x(Number(2) << o._b);
        return congruence_t(_a * x, _b * x);
      } else {
        Number x(Number(2) << o._b);
        Number y(Number(2) << o._a);
        // aZ + b << a'Z + b' = (gcd(a, b * (2^a' - 1)))*(2^b')Z + b*(2^b')
        return congruence_t(gcd(_a, _b * (y - 1)) * x, _b * x);
      }
    }
  }

  congruence_t AShr(congruence_t o) const {
    if (this->is_bottom() || o.is_bottom())
      return congruence_t::bottom();
    else if (this->is_top() || o.is_top())
      return congruence_t::top();
    else if (is_unlimited(TypeSize))
      return congruence_t::top();
    else {
      Number t = scanr1(_a);
      if (o._a == 0 && scanr1(_a) == scanl1(_a) && o._b < scanr1(_a))
        return congruence_t(Number(2) << (t - o._b), _b >> o._b);
      else {
        return congruence_t::top();
      }
    }
  }

  congruence_t LShr(congruence_t o) const {
    if (is_bottom() || o.is_bottom())
      return congruence_t::bottom();
    else
      return congruence_t::top();
  }

  // division and remainder operations

  congruence_t SDiv(congruence_t x) const { return this->operator/(x); }

  congruence_t UDiv(congruence_t x) const { return congruence_t::top(); }

  congruence_t SRem(congruence_t x) const { return this->operator%(x); }

  congruence_t URem(congruence_t x) const { return congruence_t::top(); }

  // cast operations

  congruence_t Trunc(uint64_t from, uint64_t to) const {
    if (this->is_bottom())
      return congruence_t::bottom();
    else if (this->is_top())
      return congruence_t::top();
    else
      return congruence_t(gcd(Number(1) << to, _a), _b);
  }

  congruence_t ZExt(uint64_t from, uint64_t to) const {
    return congruence_t::top();
  }

  congruence_t SExt(uint64_t from, uint64_t to) const { return *this; }

  void write(std::ostream& o) {
    if (is_bottom()) {
      o << "_|_";
      return;
    }

    if (_a == 0) {
      o << _b;
      return;
    }

    bool positive = true;
    if (_b < 0) {
      positive = false;
      _b *= -1;
    }
    o << _a << "Z" << ((positive) ? "+ " : "- ") << _b;
  }

}; // end class congruence

template < typename Number, int TypeSize >
inline congruence< Number, TypeSize > operator+(
    Number c, congruence< Number, TypeSize > x) {
  return congruence< Number, TypeSize >(c) + x;
}

template < typename Number, int TypeSize >
inline congruence< Number, TypeSize > operator+(
    congruence< Number, TypeSize > x, Number c) {
  return x + congruence< Number, TypeSize >(c);
}

template < typename Number, int TypeSize >
inline congruence< Number, TypeSize > operator*(
    Number c, congruence< Number, TypeSize > x) {
  return congruence< Number, TypeSize >(c) * x;
}

template < typename Number, int TypeSize >
inline congruence< Number, TypeSize > operator*(
    congruence< Number, TypeSize > x, Number c) {
  return x * congruence< Number, TypeSize >(c);
}

template < typename Number, int TypeSize >
inline congruence< Number, TypeSize > operator/(
    Number c, congruence< Number, TypeSize > x) {
  return congruence< Number, TypeSize >(c) / x;
}

template < typename Number, int TypeSize >
inline congruence< Number, TypeSize > operator/(
    congruence< Number, TypeSize > x, Number c) {
  return x / congruence< Number, TypeSize >(c);
}

template < typename Number, int TypeSize >
inline congruence< Number, TypeSize > operator-(
    Number c, congruence< Number, TypeSize > x) {
  return congruence< Number, TypeSize >(c) - x;
}

template < typename Number, int TypeSize >
inline congruence< Number, TypeSize > operator-(
    congruence< Number, TypeSize > x, Number c) {
  return x - congruence< Number, TypeSize >(c);
}

} // end namespace ikos

#endif // IKOS_VALUE_CONGRUENCE_HPP
