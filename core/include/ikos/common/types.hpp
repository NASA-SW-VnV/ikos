/**************************************************************************/ /**
 *
 * \file
 * \brief Basic type definitions.
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

#ifndef IKOS_TYPES_HPP
#define IKOS_TYPES_HPP

#include <iostream>
#include <memory>
#include <stdint.h>
#include <string>
#include <unordered_map>

#include <boost/container/slist.hpp>

#include <ikos/assert.hpp>
#include <ikos/exception.hpp>

namespace ikos {

/// \brief Type for unique indexes
typedef uint64_t index64_t;

/// \brief Index trait
//
//  index_traits is a structure of traits for indexable objects. These objects
//  are used e.g. by the patricia tree for organizing the objects in the tree.
//  An indexable object has the following requirements:
//   - Can return a unique index of type index64_t;
//   - Is orderable:
//     - Has operator<
//     - Has operator==
//     - Has operator!=
//
//  Based on these requirements, the trait gives the following features:
//    - The ability to give the index of the indexable object (by default, calls
//    the index() method)
//    - The ability to write the indexable object to a stream (by default, calls
//    the write() method)
//
//  The traits can be specialized to a specific type.
//  The traits can be used as:
//  \code{.cpp}
//    index64_t index =
//    ikos::index_traits<VariableName>::index(my_variable_name);
//    ikos::index_traits<VariableName>::write(my_ostream, my_variable_name);
//  \endcode
//
//  The write supports const references by default, but also supports non const
//  references for compatibility with the writeable class that gives non const
//  references.
template < typename T >
struct index_traits {
  static index64_t index(const T& tn) { return tn.index(); };
  static void write(std::ostream& o, const T& tn) { tn.write(o); };
  static void write(std::ostream& o, T& tn) {
    // TODO Change writeable classes to use a const reference
    tn.write(o);
  };
};

/// \brief Interface for writeable objects
class writeable {
public:
  virtual void write(std::ostream& o) = 0;

  virtual ~writeable() {}

}; // class writeable

inline std::ostream& operator<<(std::ostream& o, writeable& x) {
  x.write(o);
  return o;
}

/// \brief Container
template < typename Element >
class collection : public writeable {
public:
  typedef collection< Element > collection_t;

private:
  typedef boost::container::slist< Element > slist_t;

public:
  typedef typename slist_t::const_iterator iterator;

private:
  slist_t _slist;

public:
  collection() : _slist() {}

  collection(const collection_t& c) : _slist(c._slist) {}

  collection_t& operator=(const collection_t& c) {
    this->_slist = c._slist;
    return *this;
  }

  collection_t& operator+=(const Element& e) {
    this->_slist.push_front(e);
    return *this;
  }

  collection_t& operator+=(const collection_t& c) {
    for (iterator it = c.begin(); it != c.end(); ++it) {
      this->_slist.push_front(*it);
    }
    return *this;
  }

  collection_t operator+(const collection_t& c) const {
    collection_t r(*this);
    r.operator+=(c);
    return r;
  }

  iterator begin() const { return this->_slist.begin(); }

  iterator end() const { return this->_slist.end(); }

  bool empty() const { return this->_slist.empty(); }

  std::size_t size() const { return this->_slist.size(); }

  void write(std::ostream& o) {
    o << "{";
    for (iterator it = this->begin(); it != this->end();) {
      Element e = *it;
      o << e;
      ++it;
      if (it != end()) {
        o << "; ";
      }
    }
    o << "}";
  }

}; // end class collection

/// \brief Container data structure for typed variables
template < typename Type, typename VariableName >
class variable : public writeable {
public:
  typedef variable< Type, VariableName > variable_t;

private:
  VariableName _n;

public:
  variable(const VariableName& n) : _n(n) {}

  variable(const variable_t& v) : _n(v._n) {}

  variable_t& operator=(const variable_t& v) {
    this->_n = v._n;
    return *this;
  }

  const VariableName& name() const { return _n; }

  bool operator<(const variable_t& v) const {
    return index_traits< VariableName >::index(_n) <
           index_traits< VariableName >::index(v._n);
  }

  void write(std::ostream& o) { index_traits< VariableName >::write(o, _n); }

}; // end class variable

/// \brief Typed variables specialization of index_traits
template < typename Type, typename VariableName >
struct index_traits< variable< Type, VariableName > > {
  static index64_t index(const variable< Type, VariableName > v) {
    return index_traits< VariableName >::index(v.name());
  }

  static void write(std::ostream& o, variable< Type, VariableName >& v) {
    v.write(o);
  }
};

/// \brief Simple index management for strings
class string_factory {
public:
  class indexed_string : public writeable {
    friend class string_factory;

  private:
    std::shared_ptr< std::string > _s;
    index64_t _id;

  private:
    indexed_string();
    indexed_string(std::shared_ptr< std::string > s, index64_t id)
        : _s(s), _id(id) {}

  public:
    indexed_string(const indexed_string& is) : _s(is._s), _id(is._id) {}

    indexed_string& operator=(const indexed_string& is) {
      this->_s = is._s;
      this->_id = is._id;
      return *this;
    }

    const std::string& str() const { return *_s; }

    index64_t index() const { return this->_id; }

    bool operator<(const indexed_string& s) const { return this->_id < s._id; }

    bool operator==(const indexed_string& s) const {
      return this->_id == s._id;
    }

    bool operator!=(const indexed_string& s) const {
      return this->_id != s._id;
    }

    void write(std::ostream& o) { o << *_s; }

  }; // end class indexed_string

private:
  typedef std::unordered_map< std::string, indexed_string > map_t;

private:
  index64_t _next_id;
  map_t _map;

public:
  string_factory() : _next_id(1) {}

  string_factory(index64_t start_id) : _next_id(start_id) {}

  indexed_string operator[](const std::string& s) {
    map_t::iterator it = this->_map.find(s);
    if (it == this->_map.end()) {
      indexed_string is(std::shared_ptr< std::string >(new std::string(s)),
                        this->_next_id++);
      this->_map.insert(std::pair< std::string, indexed_string >(s, is));
      return is;
    } else {
      return it->second;
    }
  }

}; // end class string_factory

//! Index trait class specialization for indexed_string
template <>
struct index_traits< string_factory::indexed_string > {
  static index64_t index(const string_factory::indexed_string is) {
    return is.index();
  }

  static void write(std::ostream& o, string_factory::indexed_string is) {
    o << is.str();
  }
};

std::size_t hash_value(const string_factory::indexed_string& v) {
  return index_traits< string_factory::indexed_string >::index(v);
}

} // end namespace ikos

namespace std {

template <>
struct hash< ikos::string_factory::indexed_string > {
  std::size_t operator()(const ikos::string_factory::indexed_string& v) const {
    return ikos::hash_value(v);
  }
};

} // end namespace std

namespace ikos {

/// \brief Enumeration type for basic arithmetic operations
typedef enum {
  OP_ADDITION,
  OP_SUBTRACTION,
  OP_MULTIPLICATION,
  OP_DIVISION
} operation_t;

/// \brief Enumeration type for conversion operations
typedef enum { OP_TRUNC, OP_SEXT, OP_ZEXT } conv_operation_t;

/// \brief Enumeration type for bitwise operations
typedef enum {
  OP_AND,
  OP_OR,
  OP_XOR,
  OP_SHL,
  OP_LSHR,
  OP_ASHR
} bitwise_operation_t;

/// \brief Enumeration type for division operations
typedef enum { OP_SDIV, OP_UDIV, OP_SREM, OP_UREM } div_operation_t;

} // end namespace ikos

#endif // IKOS_TYPES_HPP
