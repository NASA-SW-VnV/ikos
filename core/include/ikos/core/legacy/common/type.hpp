/*******************************************************************************
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

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

#include <boost/container/slist.hpp>

#include <ikos/core/exception.hpp>
#include <ikos/core/semantic/dumpable.hpp>
#include <ikos/core/semantic/indexable.hpp>
#include <ikos/core/support/assert.hpp>

namespace ikos {
namespace core {

/// \brief Interface for writeable objects
class writeable {
public:
  writeable() = default;

  writeable(const writeable&) = default;

  writeable& operator=(const writeable&) = default;

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

} // end namespace core
} // end namespace ikos

namespace ikos {
namespace core {

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

} // end namespace core
} // end namespace ikos
