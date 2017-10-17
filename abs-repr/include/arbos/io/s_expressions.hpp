/*******************************************************************************
 *
 * Representation, manipulation and parsing of hash-cons'ed S-expressions.
 *
 * Author: Arnaud J. Venet
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

#ifndef ARBOS_S_EXPRESSIONS_HPP
#define ARBOS_S_EXPRESSIONS_HPP

#include <algorithm>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <list>
#include <memory>
#include <sstream>
#include <string>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/flyweight.hpp>
#include <boost/flyweight/intermodule_holder.hpp>
#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/shared_array.hpp>
#include <boost/tokenizer.hpp>

#include <arbos/common/bignums.hpp>
#include <arbos/common/types.hpp>
#include <arbos/semantics/fp.hpp>

namespace arbos {

class s_expression_error : public arbos_error {
public:
  s_expression_error(std::string msg) : arbos_error("[s_expressions] " + msg) {}

}; // class s_expression_error

typedef enum {
  INDEX64_ATOM,
  ZNUMBER_ATOM,
  QNUMBER_ATOM,
  FPNUMBER_ATOM,
  STRING_ATOM,
  BYTESEQ_ATOM
} atom_type_t;

namespace s_expression_internal {

inline char read_hex_digit(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  } else {
    throw s_expression_error("Incorrect hexadecimal character format");
  }
}

template < typename Visitor >
class internal_atom {
protected:
  std::size_t _hash_value;

public:
  typedef std::shared_ptr< internal_atom< Visitor > > atom_ptr;

public:
  operator atom_ptr() { return this->clone(); }

  virtual atom_ptr clone() = 0;

  virtual void accept(Visitor*) = 0;

  virtual void write(typename std::ostream&) = 0;

  virtual atom_type_t atom_type() = 0;

  std::size_t hash_value() { return this->_hash_value; }

  virtual ~internal_atom() {}

}; // class internal_atom

template < typename Visitor >
class internal_index64_atom : public internal_atom< Visitor > {
public:
  typedef std::shared_ptr< internal_atom< Visitor > > atom_ptr;

private:
  index64_t _index;

public:
  static std::shared_ptr< internal_index64_atom< Visitor > > parse(
      std::string s) {
    try {
      return std::shared_ptr< internal_index64_atom< Visitor > >(
          new internal_index64_atom< Visitor >(
              boost::lexical_cast< index64_t >(s)));
    } catch (const boost::bad_lexical_cast&) {
      throw s_expression_error("Incorrect index64_t format: " + s);
    }
  }

private:
  internal_index64_atom();

public:
  internal_index64_atom(index64_t index) : _index(index) {
    this->_hash_value = std::hash< index64_t >()(this->_index);
  }

  atom_ptr clone() {
    return std::shared_ptr< internal_index64_atom< Visitor > >(
        new internal_index64_atom< Visitor >(*this));
  }

  void accept(Visitor* v) { v->visit(this->_index); }

  void write(typename std::ostream& o) { o << "!" << this->_index; }

  atom_type_t atom_type() { return INDEX64_ATOM; }

  bool operator==(internal_index64_atom< Visitor > other) const {
    return this->_index == other._index;
  }

  index64_t data() { return this->_index; }

}; // class internal_index64_atom

template < typename Visitor >
class internal_z_number_atom : public internal_atom< Visitor > {
public:
  typedef std::shared_ptr< internal_atom< Visitor > > atom_ptr;

private:
  z_number _n;

public:
  static std::shared_ptr< internal_z_number_atom< Visitor > > parse(
      std::string s) {
    try {
      return std::shared_ptr< internal_z_number_atom< Visitor > >(
          new internal_z_number_atom< Visitor >(z_number(s)));
    } catch (const bignum_error&) {
      throw s_expression_error("Incorrect z_number format: " + s);
    }
  }

public:
  internal_z_number_atom(z_number n) : _n(n) {
    this->_hash_value = std::hash< z_number >()(this->_n);
  }

  atom_ptr clone() {
    return std::shared_ptr< internal_z_number_atom< Visitor > >(
        new internal_z_number_atom< Visitor >(*this));
  }

  void accept(Visitor* v) { v->visit(this->_n); }

  void write(typename std::ostream& o) { o << "#" << this->_n; }

  atom_type_t atom_type() { return ZNUMBER_ATOM; }

  bool operator==(internal_z_number_atom< Visitor > other) const {
    return this->_n == other._n;
  }

  z_number data() { return this->_n; }

}; // class internal_z_number_atom

template < typename Visitor >
class internal_q_number_atom : public internal_atom< Visitor > {
public:
  typedef std::shared_ptr< internal_atom< Visitor > > atom_ptr;

private:
  q_number _n;

public:
  static std::shared_ptr< internal_q_number_atom< Visitor > > parse(
      std::string s) {
    try {
      return std::shared_ptr< internal_q_number_atom< Visitor > >(
          new internal_q_number_atom< Visitor >(q_number(s)));
    } catch (const bignum_error&) {
      throw s_expression_error("Incorrect q_number format: " + s);
    }
  }

public:
  internal_q_number_atom(q_number n) : _n(n) {
    this->_hash_value = std::hash< q_number >()(this->_n);
  }

  atom_ptr clone() {
    return std::shared_ptr< internal_q_number_atom< Visitor > >(
        new internal_q_number_atom< Visitor >(*this));
  }

  void accept(Visitor* v) { v->visit(this->_n); }

  void write(typename std::ostream& o) { o << "%" << this->_n; }

  atom_type_t atom_type() { return QNUMBER_ATOM; }

  bool operator==(internal_q_number_atom< Visitor > other) const {
    return this->_n == other._n;
  }

  q_number data() { return this->_n; }

}; // class internal_q_number_atom

template < typename Visitor >
class internal_fp_number_atom : public internal_atom< Visitor > {
public:
  typedef std::shared_ptr< internal_atom< Visitor > > atom_ptr;

private:
  fp_number _fp;

public:
  static std::shared_ptr< internal_fp_number_atom< Visitor > > parse(
      std::string s) {
    try {
      return std::shared_ptr< internal_fp_number_atom< Visitor > >(
          new internal_fp_number_atom< Visitor >(fp_number(s)));
    } catch (const fp_error&) {
      throw s_expression_error("Incorrect fp_number format: " + s);
    }
  }

public:
  internal_fp_number_atom(fp_number fp) : _fp(fp) {
    this->_hash_value = std::hash< fp_number >()(this->_fp);
  }

  atom_ptr clone() {
    return std::shared_ptr< internal_fp_number_atom< Visitor > >(
        new internal_fp_number_atom< Visitor >(*this));
  }

  void accept(Visitor* v) { v->visit(this->_fp); }

  void write(typename std::ostream& o) { o << "^" << this->_fp; }

  atom_type_t atom_type() { return FPNUMBER_ATOM; }

  bool operator==(internal_fp_number_atom< Visitor > other) const {
    return this->_fp == other._fp;
  }

  fp_number data() { return this->_fp; }

}; // class internal_fp_number_atom

template < typename Visitor >
class internal_string_atom : public internal_atom< Visitor > {
public:
  typedef std::shared_ptr< internal_atom< Visitor > > atom_ptr;

private:
  std::string _s;

public:
  static std::shared_ptr< internal_string_atom< Visitor > > parse(
      std::string s) {
    std::string content;
    content.reserve(s.size());

    for (std::string::iterator it = s.begin(); it != s.end();) {
      if (*it != '\\') {
        content.push_back(*it++);
      } else {
        ++it;
        if (it == s.end()) {
          throw s_expression_error(
              "Incorrect string format in string_atom constructor [1]");
        }
        char c = 16 * read_hex_digit(*it++);
        if (it == s.end()) {
          throw s_expression_error(
              "Incorrect string format in string_atom constructor [2]");
        }
        c += read_hex_digit(*it++);
        content.push_back(c);
      }
    }

    return std::shared_ptr< internal_string_atom< Visitor > >(
        new internal_string_atom< Visitor >(content));
  }

public:
  internal_string_atom(std::string s) : _s(s) {
    this->_hash_value = std::hash< std::string >()(this->_s);
  }

  atom_ptr clone() {
    return std::shared_ptr< internal_string_atom< Visitor > >(
        new internal_string_atom< Visitor >(*this));
  }

  void accept(Visitor* v) { v->visit(this->_s); }

  void write(typename std::ostream& o) {
    std::ios_base::fmtflags previous_flags = o.flags();
    std::streamsize previous_precision = o.precision();
    char previous_fill = o.fill();

    o << "$" << std::setfill('0');
    for (typename std::string::iterator it = this->_s.begin();
         it != this->_s.end();
         ++it) {
      char c = *it;
      if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
          (c >= 'A' && c <= 'Z') || (c == '_') || (c == '.') || (c == '*') ||
          (c == ':') || (c == '-') || (c == '/')) {
        o << c;
      } else {
        o << "\\" << std::hex << std::uppercase << std::setw(2) << (int)c
          << std::dec << std::nouppercase;
      }
    }

    o.flags(previous_flags);
    o.precision(previous_precision);
    o.fill(previous_fill);
  }

  atom_type_t atom_type() { return STRING_ATOM; }

  bool operator==(internal_string_atom< Visitor > other) const {
    return this->_s == other._s;
  }

  std::string data() { return this->_s; }

}; // class internal_string_atom

template < typename Visitor >
class internal_byte_sequence_atom : public internal_atom< Visitor > {
public:
  typedef std::shared_ptr< internal_atom< Visitor > > atom_ptr;

private:
  std::vector< char > _b;

public:
  static std::shared_ptr< internal_byte_sequence_atom< Visitor > > parse(
      std::string s) {
    std::vector< char > seq;
    seq.reserve(s.size() / 3);

    if (s.size() % 3 != 0) {
      throw s_expression_error(
          "Incorrect string format in byte_sequence_atom constructor [1]");
    }
    bool in_sequence = false;
    for (std::string::iterator it = s.begin(); it != s.end();) {
      char c;
      in_sequence = true;
      if (*it != '\\') {
        throw s_expression_error(
            "Incorrect string format in byte_sequence_atom constructor [2]");
      }
      ++it;
      if (it == s.end()) {
        throw s_expression_error(
            "Incorrect string format in byte_sequence_atom constructor [3]");
      }
      c = 16 * read_hex_digit(*it++);
      if (it == s.end()) {
        throw s_expression_error(
            "Incorrect string format in byte_sequence_atom constructor [4]");
      }
      c += read_hex_digit(*it++);
      in_sequence = false;
      seq.push_back(c);
    }
    if (in_sequence) {
      throw s_expression_error(
          "Incorrect string format in byte_sequence_atom constructor [5]");
    }
    return std::shared_ptr< internal_byte_sequence_atom< Visitor > >(
        new internal_byte_sequence_atom< Visitor >(seq));
  }

public:
  internal_byte_sequence_atom(std::vector< char > b) : _b(b) {
    std::size_t seed = 0;
    for (std::vector< char >::iterator it = this->_b.begin();
         it != this->_b.end();
         ++it) {
      boost::hash_combine(seed, *it);
    }
    this->_hash_value = seed;
  }

  atom_ptr clone() {
    return std::shared_ptr< internal_byte_sequence_atom< Visitor > >(
        new internal_byte_sequence_atom< Visitor >(*this));
  }

  void accept(Visitor* v) { v->visit(this->_b); }

  void write(typename std::ostream& o) {
    std::ios_base::fmtflags previous_flags = o.flags();
    std::streamsize previous_precision = o.precision();
    char previous_fill = o.fill();

    o << "[" << std::setfill('0');
    for (typename std::vector< char >::iterator it = this->_b.begin();
         it != this->_b.end();
         ++it) {
      o << "\\" << std::hex << std::uppercase << std::setw(2) << (int)*it
        << std::dec << std::nouppercase;
    }

    o.flags(previous_flags);
    o.precision(previous_precision);
    o.fill(previous_fill);
  }

  atom_type_t atom_type() { return BYTESEQ_ATOM; }

  bool operator==(internal_byte_sequence_atom< Visitor > other) const {
    return this->_b == other._b;
  }

  std::vector< char > data() { return this->_b; }

}; // class byte_sequence_atom

} // namespace s_expression_internal

class atom_visitor {
public:
  virtual void visit(index64_t&) = 0;

  virtual void visit(z_number&) = 0;

  virtual void visit(q_number&) = 0;

  virtual void visit(fp_number&) = 0;

  virtual void visit(std::string&) = 0;

  virtual void visit(std::vector< char >&) = 0;

  virtual ~atom_visitor() {}

}; // class atom_visitor

typedef s_expression_internal::internal_atom< atom_visitor > atom;
typedef s_expression_internal::internal_index64_atom< atom_visitor >
    index64_atom;
typedef s_expression_internal::internal_z_number_atom< atom_visitor >
    z_number_atom;
typedef s_expression_internal::internal_q_number_atom< atom_visitor >
    q_number_atom;
typedef s_expression_internal::internal_fp_number_atom< atom_visitor >
    fp_number_atom;
typedef s_expression_internal::internal_string_atom< atom_visitor > string_atom;
typedef s_expression_internal::internal_byte_sequence_atom< atom_visitor >
    byte_sequence_atom;

inline std::ostream& operator<<(std::ostream& o, atom& a) {
  a.write(o);
  return o;
}

inline std::size_t hash_value(const index64_atom& a_) {
  index64_atom& a = const_cast< index64_atom& >(a_);
  return a.hash_value();
}

inline std::size_t hash_value(const z_number_atom& a_) {
  z_number_atom& a = const_cast< z_number_atom& >(a_);
  return a.hash_value();
}

inline std::size_t hash_value(const q_number_atom& a_) {
  q_number_atom& a = const_cast< q_number_atom& >(a_);
  return a.hash_value();
}

inline std::size_t hash_value(const fp_number_atom& a_) {
  fp_number_atom& a = const_cast< fp_number_atom& >(a_);
  return a.hash_value();
}

inline std::size_t hash_value(const string_atom& a_) {
  string_atom& a = const_cast< string_atom& >(a_);
  return a.hash_value();
}

inline std::size_t hash_value(const byte_sequence_atom& a_) {
  byte_sequence_atom& a = const_cast< byte_sequence_atom& >(a_);
  return a.hash_value();
}

namespace s_expression_internal {

class s_expr_atom {
public:
  typedef atom::atom_ptr atom_ptr;

private:
  atom_ptr _atom;

public:
  s_expr_atom(atom_ptr atom) : _atom(atom) {}

  atom_ptr get() { return this->_atom; }

  bool operator==(s_expr_atom other) const {
    if (this->_atom->atom_type() == INDEX64_ATOM &&
        other._atom->atom_type() == INDEX64_ATOM) {
      std::shared_ptr< index64_atom > a1 =
          std::static_pointer_cast< index64_atom, atom >(this->_atom);
      std::shared_ptr< index64_atom > a2 =
          std::static_pointer_cast< index64_atom, atom >(other._atom);
      return a1->operator==(*a2);
    } else if (this->_atom->atom_type() == ZNUMBER_ATOM &&
               other._atom->atom_type() == ZNUMBER_ATOM) {
      std::shared_ptr< z_number_atom > a1 =
          std::static_pointer_cast< z_number_atom, atom >(this->_atom);
      std::shared_ptr< z_number_atom > a2 =
          std::static_pointer_cast< z_number_atom, atom >(other._atom);
      return a1->operator==(*a2);
    } else if (this->_atom->atom_type() == QNUMBER_ATOM &&
               other._atom->atom_type() == QNUMBER_ATOM) {
      std::shared_ptr< q_number_atom > a1 =
          std::static_pointer_cast< q_number_atom, atom >(this->_atom);
      std::shared_ptr< q_number_atom > a2 =
          std::static_pointer_cast< q_number_atom, atom >(other._atom);
      return a1->operator==(*a2);
    } else if (this->_atom->atom_type() == FPNUMBER_ATOM &&
               other._atom->atom_type() == FPNUMBER_ATOM) {
      std::shared_ptr< fp_number_atom > a1 =
          std::static_pointer_cast< fp_number_atom, atom >(this->_atom);
      std::shared_ptr< fp_number_atom > a2 =
          std::static_pointer_cast< fp_number_atom, atom >(other._atom);
      return a1->operator==(*a2);
    } else if (this->_atom->atom_type() == STRING_ATOM &&
               other._atom->atom_type() == STRING_ATOM) {
      std::shared_ptr< string_atom > a1 =
          std::static_pointer_cast< string_atom, atom >(this->_atom);
      std::shared_ptr< string_atom > a2 =
          std::static_pointer_cast< string_atom, atom >(other._atom);
      return a1->operator==(*a2);
    } else if (this->_atom->atom_type() == BYTESEQ_ATOM &&
               other._atom->atom_type() == BYTESEQ_ATOM) {
      std::shared_ptr< byte_sequence_atom > a1 =
          std::static_pointer_cast< byte_sequence_atom, atom >(this->_atom);
      std::shared_ptr< byte_sequence_atom > a2 =
          std::static_pointer_cast< byte_sequence_atom, atom >(other._atom);
      return a1->operator==(*a2);
    } else {
      return false;
    }
  }

  std::size_t hash_value() { return this->_atom->hash_value(); }

}; // class s_expr_atom

inline std::size_t hash_value(const s_expr_atom& a_) {
  s_expr_atom& a = const_cast< s_expr_atom& >(a_);
  return a.hash_value();
}

class s_expr;
typedef boost::flyweight< s_expr_atom, boost::flyweights::intermodule_holder >
    hashed_s_expr_atom;
typedef boost::flyweight< s_expr, boost::flyweights::intermodule_holder >
    hashed_s_expr;
typedef std::list< hashed_s_expr > hashed_s_expr_list;

class s_expr {
public:
  typedef atom::atom_ptr atom_ptr;

private:
  typedef boost::optional< hashed_s_expr > placeholder;
  typedef boost::shared_array< placeholder > hashed_s_expr_array;

private:
  std::size_t _n_args;
  hashed_s_expr_atom _functor;
  hashed_s_expr_array _arguments;
  std::size_t _hash_value;

public:
  s_expr(atom_ptr functor, hashed_s_expr_list arguments)
      : _n_args(arguments.size()),
        _functor(functor),
        _arguments(new placeholder[arguments.size()]) {
    std::size_t seed = functor->hash_value();
    std::size_t i = 0;
    for (hashed_s_expr_list::iterator it = arguments.begin();
         it != arguments.end();
         ++it, ++i) {
      this->_arguments[i] = placeholder(*it);
      boost::hash_combine(seed, *it);
    }
    this->_hash_value = seed;
  }

  std::size_t n_args() { return this->_n_args; }

  hashed_s_expr_atom functor() { return this->_functor; }

  hashed_s_expr argument(std::size_t n) {
    if (n > 0 && n <= this->_n_args) {
      placeholder arg = this->_arguments[n - 1];
      if (arg) {
        return *arg;
      } else {
        std::ostringstream buf;
        buf << "Uninitialized argument in S-expression [1]: " << n;
        throw s_expression_error(buf.str());
      }
    } else {
      std::ostringstream buf;
      buf << "Out-of-bounds S-expression argument: " << n;
      throw s_expression_error(buf.str());
    }
  }

  bool is_atomic() { return this->_n_args == 0; }

  bool operator==(s_expr other) const {
    if (this->_n_args == other._n_args) {
      if (this->_functor == other._functor) {
        for (std::size_t i = 0; i < this->_n_args; ++i) {
          placeholder a1 = this->_arguments[i];
          placeholder a2 = other._arguments[i];
          if (a1 && a2) {
            if (*a1 != *a2) {
              return false;
            }
          } else {
            std::ostringstream buf;
            buf << "Uninitialized argument in S-expression [2]: " << i;
            throw s_expression_error(buf.str());
          }
        }
        return true;
      } else {
        return false;
      }
    } else {
      return false;
    }
  }

  std::size_t hash_value() { return this->_hash_value; }

  void write(std::ostream& o) {
    s_expr_atom& a = const_cast< s_expr_atom& >(this->_functor.get());
    o << "(" << *(a.get());
    for (std::size_t i = 0; i < this->_n_args; ++i) {
      o << " ";
      placeholder arg = this->_arguments[i];
      if (arg) {
        s_expr& a = const_cast< s_expr& >(arg->get());
        a.write(o);
      } else {
        std::ostringstream buf;
        buf << "Uninitialized argument in S-expression [3]: " << i;
        throw s_expression_error(buf.str());
      }
    }
    o << ")";
  }

  void write_formatted(std::size_t tab,
                       std::ostream& o,
                       std::set< std::string >& breaks) {
    bool indent = false;
    s_expr_atom& a = const_cast< s_expr_atom& >(this->_functor.get());
    atom& at = *(a.get());
    o << "(" << at;
    if (at.atom_type() == STRING_ATOM) {
      string_atom& str = static_cast< string_atom& >(at);
      if (breaks.find(str.data()) != breaks.end()) {
        indent = true;
        tab += 2;
      }
    }
    for (std::size_t i = 0; i < this->_n_args; ++i) {
      if (indent) {
        o << std::endl << std::setw(tab);
      } else {
        o << " ";
      }
      placeholder arg = this->_arguments[i];
      if (arg) {
        s_expr& a = const_cast< s_expr& >(arg->get());
        a.write_formatted(tab, o, breaks);
      } else {
        std::ostringstream buf;
        buf << "Uninitialized argument in S-expression [4]: " << i;
        throw s_expression_error(buf.str());
      }
    }
    if (indent) {
      o << std::endl << std::setw(tab - 2);
    }
    o << ")";
  }

}; // class s_expr

inline std::size_t hash_value(const s_expr& e_) {
  s_expr& e = const_cast< s_expr& >(e_);
  return e.hash_value();
}

} // namespace s_expression_internal

class s_expression {
  friend class s_expression_istream;
  friend class s_expression_ostream;
  friend class s_expression_formatter;

private:
  typedef s_expression_internal::s_expr s_expr;
  typedef s_expression_internal::s_expr_atom s_expr_atom;
  typedef s_expression_internal::hashed_s_expr hashed_s_expr;

private:
  hashed_s_expr _hashed_s_expr;

private:
  s_expression();

  s_expression(hashed_s_expr hashed_s_expr) : _hashed_s_expr(hashed_s_expr) {}

  s_expression(s_expr s_expr) : _hashed_s_expr(s_expr) {}

  void write_formatted(std::ostream& o, std::set< std::string >& breaks) {
    s_expr& e = const_cast< s_expr& >(this->_hashed_s_expr.get());
    e.write_formatted(0, o, breaks);
  }

public:
  void write(std::ostream& o) {
    s_expr& e = const_cast< s_expr& >(this->_hashed_s_expr.get());
    e.write(o);
  }

  atom& operator*() {
    s_expr& e = const_cast< s_expr& >(this->_hashed_s_expr.get());
    s_expr_atom& a = const_cast< s_expr_atom& >(e.functor().get());
    return *(a.get());
  }

  s_expression operator[](std::size_t n) {
    s_expr& e = const_cast< s_expr& >(this->_hashed_s_expr.get());
    return s_expression(e.argument(n));
  }

  std::size_t n_args() {
    s_expr& e = const_cast< s_expr& >(this->_hashed_s_expr.get());
    return e.n_args();
  }

  bool is_atomic() {
    s_expr& e = const_cast< s_expr& >(this->_hashed_s_expr.get());
    return e.is_atomic();
  }

}; // class s_expression

inline std::ostream& operator<<(std::ostream& o, s_expression e) {
  e.write(o);
  return o;
}

typedef boost::optional< s_expression > s_expression_ref;

namespace s_expression_internal {

class s_pattern_internal {
public:
  virtual bool operator^(s_expression e) = 0;

  virtual void sanitize() = 0;

  virtual ~s_pattern_internal() {}

}; // class s_pattern_internal

typedef std::shared_ptr< s_pattern_internal > s_pattern_internal_ptr;

class s_any_pattern_internal : public s_pattern_internal {
public:
  s_any_pattern_internal() {}

  void sanitize() {}

  bool operator^(s_expression) { return true; }

}; // class s_any_pattern_internal

class s_simple_pattern_internal : public s_pattern_internal {
private:
  s_expression_ref& _v;

private:
  s_simple_pattern_internal();

public:
  s_simple_pattern_internal(s_expression_ref& v) : _v(v) {}

  void sanitize() { this->_v = s_expression_ref(); }

  bool operator^(s_expression e) {
    this->_v = e;
    return true;
  }

}; // class s_simple_pattern_internal

class s_complex_pattern_internal : public s_pattern_internal {
private:
  std::string _functor;
  std::vector< s_pattern_internal_ptr > _args;

private:
  s_complex_pattern_internal();

public:
  s_complex_pattern_internal(
      std::string functor,
      s_pattern_internal_ptr p1 = s_pattern_internal_ptr(),
      s_pattern_internal_ptr p2 = s_pattern_internal_ptr(),
      s_pattern_internal_ptr p3 = s_pattern_internal_ptr(),
      s_pattern_internal_ptr p4 = s_pattern_internal_ptr(),
      s_pattern_internal_ptr p5 = s_pattern_internal_ptr(),
      s_pattern_internal_ptr p6 = s_pattern_internal_ptr(),
      s_pattern_internal_ptr p7 = s_pattern_internal_ptr(),
      s_pattern_internal_ptr p8 = s_pattern_internal_ptr(),
      s_pattern_internal_ptr p9 = s_pattern_internal_ptr(),
      s_pattern_internal_ptr p10 = s_pattern_internal_ptr())
      : _functor(functor) {
    if (p1) {
      this->_args.push_back(p1);
    }
    if (p2) {
      this->_args.push_back(p2);
    }
    if (p3) {
      this->_args.push_back(p3);
    }
    if (p4) {
      this->_args.push_back(p4);
    }
    if (p5) {
      this->_args.push_back(p5);
    }
    if (p6) {
      this->_args.push_back(p6);
    }
    if (p7) {
      this->_args.push_back(p7);
    }
    if (p8) {
      this->_args.push_back(p8);
    }
    if (p9) {
      this->_args.push_back(p9);
    }
    if (p10) {
      this->_args.push_back(p10);
    }
  }

  void sanitize() {
    for (std::vector< s_pattern_internal_ptr >::iterator it =
             this->_args.begin();
         it != this->_args.end();
         ++it) {
      (*it)->sanitize();
    }
  }

  bool operator^(s_expression e) {
    if (this->_args.size() == 0 || (this->_args.size() == e.n_args())) {
      atom& f = *e;
      if (f.atom_type() == STRING_ATOM) {
        string_atom& a = static_cast< string_atom& >(f);
        if (boost::equal(a.data(), this->_functor)) {
          if (this->_args.size() != 0) {
            for (std::size_t i = 0; i < e.n_args(); ++i) {
              if (!this->_args[i]->operator^(e[i + 1])) {
                return false;
              }
            }
          }
          return true;
        } else {
          return false;
        }
      } else {
        return false;
      }
    } else {
      return false;
    }
  }

}; // class s_complex_pattern_internal

} // namespace s_expression_internal

class s_pattern {
private:
  typedef s_expression_internal::s_pattern_internal_ptr s_pattern_internal_ptr;
  typedef s_expression_internal::s_any_pattern_internal s_any_pattern_internal;
  typedef s_expression_internal::s_simple_pattern_internal
      s_simple_pattern_internal;
  typedef s_expression_internal::s_complex_pattern_internal
      s_complex_pattern_internal;

private:
  s_pattern_internal_ptr _pattern;

public:
  s_pattern()
      : _pattern(s_pattern_internal_ptr(new s_any_pattern_internal())) {}

  s_pattern(s_expression_ref& v)
      : _pattern(s_pattern_internal_ptr(new s_simple_pattern_internal(v))) {}

  s_pattern(std::string functor)
      : _pattern(
            s_pattern_internal_ptr(new s_complex_pattern_internal(functor))) {}

  s_pattern(const char* functor)
      : _pattern(
            s_pattern_internal_ptr(new s_complex_pattern_internal(functor))) {}

  s_pattern(std::string functor, s_pattern p1)
      : _pattern(s_pattern_internal_ptr(
            new s_complex_pattern_internal(functor, p1._pattern))) {}

  s_pattern(const char* functor, s_pattern p1)
      : _pattern(s_pattern_internal_ptr(
            new s_complex_pattern_internal(functor, p1._pattern))) {}

  s_pattern(std::string functor, s_pattern p1, s_pattern p2)
      : _pattern(s_pattern_internal_ptr(
            new s_complex_pattern_internal(functor,
                                           p1._pattern,
                                           p2._pattern))) {}

  s_pattern(const char* functor, s_pattern p1, s_pattern p2)
      : _pattern(s_pattern_internal_ptr(
            new s_complex_pattern_internal(functor,
                                           p1._pattern,
                                           p2._pattern))) {}

  s_pattern(std::string functor, s_pattern p1, s_pattern p2, s_pattern p3)
      : _pattern(s_pattern_internal_ptr(
            new s_complex_pattern_internal(functor,
                                           p1._pattern,
                                           p2._pattern,
                                           p3._pattern))) {}

  s_pattern(const char* functor, s_pattern p1, s_pattern p2, s_pattern p3)
      : _pattern(s_pattern_internal_ptr(
            new s_complex_pattern_internal(functor,
                                           p1._pattern,
                                           p2._pattern,
                                           p3._pattern))) {}

  s_pattern(std::string functor,
            s_pattern p1,
            s_pattern p2,
            s_pattern p3,
            s_pattern p4)
      : _pattern(s_pattern_internal_ptr(
            new s_complex_pattern_internal(functor,
                                           p1._pattern,
                                           p2._pattern,
                                           p3._pattern,
                                           p4._pattern))) {}

  s_pattern(const char* functor,
            s_pattern p1,
            s_pattern p2,
            s_pattern p3,
            s_pattern p4)
      : _pattern(s_pattern_internal_ptr(
            new s_complex_pattern_internal(functor,
                                           p1._pattern,
                                           p2._pattern,
                                           p3._pattern,
                                           p4._pattern))) {}

  s_pattern(std::string functor,
            s_pattern p1,
            s_pattern p2,
            s_pattern p3,
            s_pattern p4,
            s_pattern p5)
      : _pattern(s_pattern_internal_ptr(
            new s_complex_pattern_internal(functor,
                                           p1._pattern,
                                           p2._pattern,
                                           p3._pattern,
                                           p4._pattern,
                                           p5._pattern))) {}

  s_pattern(const char* functor,
            s_pattern p1,
            s_pattern p2,
            s_pattern p3,
            s_pattern p4,
            s_pattern p5)
      : _pattern(s_pattern_internal_ptr(
            new s_complex_pattern_internal(functor,
                                           p1._pattern,
                                           p2._pattern,
                                           p3._pattern,
                                           p4._pattern,
                                           p5._pattern))) {}

  s_pattern(std::string functor,
            s_pattern p1,
            s_pattern p2,
            s_pattern p3,
            s_pattern p4,
            s_pattern p5,
            s_pattern p6)
      : _pattern(s_pattern_internal_ptr(
            new s_complex_pattern_internal(functor,
                                           p1._pattern,
                                           p2._pattern,
                                           p3._pattern,
                                           p4._pattern,
                                           p5._pattern,
                                           p6._pattern))) {}

  s_pattern(const char* functor,
            s_pattern p1,
            s_pattern p2,
            s_pattern p3,
            s_pattern p4,
            s_pattern p5,
            s_pattern p6)
      : _pattern(s_pattern_internal_ptr(
            new s_complex_pattern_internal(functor,
                                           p1._pattern,
                                           p2._pattern,
                                           p3._pattern,
                                           p4._pattern,
                                           p5._pattern,
                                           p6._pattern))) {}

  s_pattern(std::string functor,
            s_pattern p1,
            s_pattern p2,
            s_pattern p3,
            s_pattern p4,
            s_pattern p5,
            s_pattern p6,
            s_pattern p7)
      : _pattern(s_pattern_internal_ptr(
            new s_complex_pattern_internal(functor,
                                           p1._pattern,
                                           p2._pattern,
                                           p3._pattern,
                                           p4._pattern,
                                           p5._pattern,
                                           p6._pattern,
                                           p7._pattern))) {}

  s_pattern(const char* functor,
            s_pattern p1,
            s_pattern p2,
            s_pattern p3,
            s_pattern p4,
            s_pattern p5,
            s_pattern p6,
            s_pattern p7)
      : _pattern(s_pattern_internal_ptr(
            new s_complex_pattern_internal(functor,
                                           p1._pattern,
                                           p2._pattern,
                                           p3._pattern,
                                           p4._pattern,
                                           p5._pattern,
                                           p6._pattern,
                                           p7._pattern))) {}

  s_pattern(std::string functor,
            s_pattern p1,
            s_pattern p2,
            s_pattern p3,
            s_pattern p4,
            s_pattern p5,
            s_pattern p6,
            s_pattern p7,
            s_pattern p8)
      : _pattern(s_pattern_internal_ptr(
            new s_complex_pattern_internal(functor,
                                           p1._pattern,
                                           p2._pattern,
                                           p3._pattern,
                                           p4._pattern,
                                           p5._pattern,
                                           p6._pattern,
                                           p7._pattern,
                                           p8._pattern))) {}

  s_pattern(const char* functor,
            s_pattern p1,
            s_pattern p2,
            s_pattern p3,
            s_pattern p4,
            s_pattern p5,
            s_pattern p6,
            s_pattern p7,
            s_pattern p8)
      : _pattern(s_pattern_internal_ptr(
            new s_complex_pattern_internal(functor,
                                           p1._pattern,
                                           p2._pattern,
                                           p3._pattern,
                                           p4._pattern,
                                           p5._pattern,
                                           p6._pattern,
                                           p7._pattern,
                                           p8._pattern))) {}

  s_pattern(std::string functor,
            s_pattern p1,
            s_pattern p2,
            s_pattern p3,
            s_pattern p4,
            s_pattern p5,
            s_pattern p6,
            s_pattern p7,
            s_pattern p8,
            s_pattern p9)
      : _pattern(s_pattern_internal_ptr(
            new s_complex_pattern_internal(functor,
                                           p1._pattern,
                                           p2._pattern,
                                           p3._pattern,
                                           p4._pattern,
                                           p5._pattern,
                                           p6._pattern,
                                           p7._pattern,
                                           p8._pattern,
                                           p9._pattern))) {}

  s_pattern(const char* functor,
            s_pattern p1,
            s_pattern p2,
            s_pattern p3,
            s_pattern p4,
            s_pattern p5,
            s_pattern p6,
            s_pattern p7,
            s_pattern p8,
            s_pattern p9)
      : _pattern(s_pattern_internal_ptr(
            new s_complex_pattern_internal(functor,
                                           p1._pattern,
                                           p2._pattern,
                                           p3._pattern,
                                           p4._pattern,
                                           p5._pattern,
                                           p6._pattern,
                                           p7._pattern,
                                           p8._pattern,
                                           p9._pattern))) {}

  s_pattern(std::string functor,
            s_pattern p1,
            s_pattern p2,
            s_pattern p3,
            s_pattern p4,
            s_pattern p5,
            s_pattern p6,
            s_pattern p7,
            s_pattern p8,
            s_pattern p9,
            s_pattern p10)
      : _pattern(s_pattern_internal_ptr(
            new s_complex_pattern_internal(functor,
                                           p1._pattern,
                                           p2._pattern,
                                           p3._pattern,
                                           p4._pattern,
                                           p5._pattern,
                                           p6._pattern,
                                           p7._pattern,
                                           p8._pattern,
                                           p9._pattern,
                                           p10._pattern))) {}

  s_pattern(const char* functor,
            s_pattern p1,
            s_pattern p2,
            s_pattern p3,
            s_pattern p4,
            s_pattern p5,
            s_pattern p6,
            s_pattern p7,
            s_pattern p8,
            s_pattern p9,
            s_pattern p10)
      : _pattern(s_pattern_internal_ptr(
            new s_complex_pattern_internal(functor,
                                           p1._pattern,
                                           p2._pattern,
                                           p3._pattern,
                                           p4._pattern,
                                           p5._pattern,
                                           p6._pattern,
                                           p7._pattern,
                                           p8._pattern,
                                           p9._pattern,
                                           p10._pattern))) {}

  bool operator^(s_expression e) {
    bool matching = this->_pattern->operator^(e);
    if (!matching) {
      this->_pattern->sanitize();
    }
    return matching;
  }

}; // class s_pattern

class s_expression_istream {
private:
  typedef boost::tokenizer< boost::char_separator< char >,
                            std::istreambuf_iterator< char > > tokenizer_t;
  typedef atom::atom_ptr atom_ptr;
  typedef s_expression_internal::s_expr s_expr;
  typedef s_expression_internal::hashed_s_expr hashed_s_expr;
  typedef s_expression_internal::hashed_s_expr_list hashed_s_expr_list;

private:
  tokenizer_t _tokenizer;
  tokenizer_t::iterator _it;

private:
  void skip_token(std::string token) {
    if (this->_it == this->_tokenizer.end() || *this->_it++ != token) {
      std::ostringstream buf;
      buf << "Expected token " << token << " in S-expression";
      throw s_expression_error(buf.str());
    }
  }

  atom_ptr parse_atom() {
    std::string atom = *this->_it++;
    if (atom.empty()) {
      throw s_expression_error("Empty atom [1]");
    } else {
      char header = atom.at(0);
      std::string contents = atom.substr(1);
      if (contents.empty()) {
        throw s_expression_error("Empty atom [2]");
      }
      if (header == '!') {
        return index64_atom::parse(contents);
      } else if (header == '#') {
        return z_number_atom::parse(contents);
      } else if (header == '%') {
        return q_number_atom::parse(contents);
      } else if (header == '^') {
        return fp_number_atom::parse(contents);
      } else if (header == '$') {
        return string_atom::parse(contents);
      } else if (header == '[') {
        return byte_sequence_atom::parse(contents);
      } else {
        std::ostringstream buf;
        buf << "Unknown atom header: " << header;
        throw s_expression_error(buf.str());
      }
    }
  }

  hashed_s_expr parse_s_expr() {
    atom_ptr functor;
    hashed_s_expr_list arguments;

    if (this->_it == this->_tokenizer.end()) {
      throw s_expression_error("Malformed S-expression [1]");
    } else {
      this->skip_token("(");
      if (this->_it == this->_tokenizer.end() || *this->_it == "(" ||
          *this->_it == ")") {
        // First element must be an atom
        throw s_expression_error("Malformed S-expression [2]: " + *this->_it);
      } else {
        functor = this->parse_atom();
        while (this->_it != this->_tokenizer.end() && *this->_it != ")") {
          arguments.push_back(this->parse_s_expr());
        }
        if (this->_it == this->_tokenizer.end()) {
          throw s_expression_error("Malformed S-expression [3]");
        } else {
          this->skip_token(")");
        }
      }
    }
    return hashed_s_expr(s_expr(functor, arguments));
  }

  s_expression parse_s_expression() {
    return s_expression(this->parse_s_expr());
  }

public:
  s_expression_istream(std::istream& instream)
      : _tokenizer(std::istreambuf_iterator< char >(instream),
                   std::istreambuf_iterator< char >(),
                   boost::char_separator< char >(" \t\r\n", "()")),
        _it(_tokenizer.begin()) {}

  s_expression_istream& operator>>(s_expression_ref& e) {
    if (this->_it != this->_tokenizer.end()) {
      e = s_expression_ref(s_expression(this->parse_s_expression()));
    }
    return *this;
  }

}; // class s_expression_istream

class s_expression_ostream {
private:
  typedef atom::atom_ptr atom_ptr;
  typedef s_expression_internal::s_expr s_expr;
  typedef s_expression_internal::hashed_s_expr hashed_s_expr;
  typedef s_expression_internal::hashed_s_expr_list hashed_s_expr_list;

private:
  atom_ptr _functor;
  hashed_s_expr_list _arguments;

private:
  s_expression_ostream();

public:
  s_expression_ostream(atom_ptr functor) : _functor(functor) {}

  s_expression_ostream(const char* s) : _functor(string_atom(s)) {}

  s_expression_ostream& operator<<(s_expression e) {
    this->_arguments.push_back(e._hashed_s_expr);
    return *this;
  }

  s_expression_ostream& operator<<(atom_ptr a) {
    hashed_s_expr_list args;
    this->_arguments.push_back(hashed_s_expr(s_expr(a, args)));
    return *this;
  }

  s_expression expr() {
    return s_expression(s_expr(this->_functor, this->_arguments));
  }

  std::string str() {
    std::ostringstream buf;
    buf << this->expr();
    return buf.str();
  }

}; // class s_expression_ostream

class sexpr_indent {
  friend class s_expression_formatter;

private:
  std::string _break_label;

private:
  sexpr_indent();

public:
  sexpr_indent(std::string break_label) : _break_label(break_label) {}

}; // class sexpr_indent

class s_expression_formatter {
private:
  std::ostream& _out;
  std::set< std::string > _break_labels;

private:
  s_expression_formatter();

public:
  s_expression_formatter(std::ostream& out) : _out(out) {}

  s_expression_formatter& operator<<(sexpr_indent i) {
    this->_break_labels.insert(i._break_label);
    return *this;
  }

  s_expression_formatter& operator<<(s_expression e) {
    e.write_formatted(this->_out, this->_break_labels);
    return *this;
  }

}; // class s_expresion_formatter

} // namespace arbos

#endif // ARBOS_S_EXPRESSIONS_HPP
