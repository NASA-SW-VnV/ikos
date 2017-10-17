/*******************************************************************************
 *
 * Basic type definitions.
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

#ifndef ARBOS_TYPES_HPP
#define ARBOS_TYPES_HPP

#include <stdint.h>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

namespace arbos {

typedef uint64_t index64_t;

class arbos_error {
private:
  std::string _msg;

private:
  arbos_error();

public:
  arbos_error(std::string msg) : _msg(msg) {}

  virtual std::string message() { return this->_msg; }

  virtual void write(std::ostream& o) { o << this->message(); }

  virtual ~arbos_error() {}

}; // class arbos_error

inline std::ostream& operator<<(std::ostream& o, arbos_error& e) {
  e.write(o);
  return o;
}

// Interface for writeable objects
class writeable {
public:
  virtual void write(std::ostream& o) = 0;

  virtual ~writeable() {}

}; // class writeable

inline std::ostream& operator<<(std::ostream& o, writeable& x) {
  x.write(o);
  return o;
}

// Container data structure for typed variables
template < typename Type, typename VariableName >
class variable : public writeable {
public:
  typedef variable< Type, VariableName > variable_t;

public:
  VariableName _n;

public:
  variable(VariableName n) : _n(n) {}

  variable(const variable_t& v) : writeable(), _n(v._n) {}

  variable_t& operator=(variable_t v) {
    this->_n = v._n;
    return *this;
  }

  VariableName name() { return _n; }

  index64_t index() { return _n.index(); }

  bool operator<(const variable_t& v) const {
    variable_t v1 = const_cast< variable_t& >(*this);
    variable_t v2 = const_cast< variable_t& >(v);
    return v1._n.index() < v2._n.index();
  }

  void write(std::ostream& o) { o << _n; }

}; // class variable

// Simple index management for strings
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
    indexed_string(const indexed_string& is)
        : writeable(), _s(is._s), _id(is._id) {}

    indexed_string& operator=(indexed_string is) {
      this->_s = is._s;
      this->_id = is._id;
      return *this;
    }

    index64_t index() { return this->_id; }

    bool operator<(indexed_string s) { return (this->_id < s._id); }

    bool operator==(indexed_string s) { return (this->_id == s._id); }

    void write(std::ostream& o) { o << *_s; }

  }; // class indexed_string

private:
  typedef std::unordered_map< std::string, indexed_string > map_t;

private:
  index64_t _next_id;
  map_t _map;

public:
  string_factory() : _next_id(1) {}

  string_factory(index64_t start_id) : _next_id(start_id) {}

  indexed_string operator[](std::string s) {
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

}; // class string_factory

// Enumeration type for basic arithmetic operations
typedef enum {
  OP_ADDITION,
  OP_SUBTRACTION,
  OP_MULTIPLICATION,
  OP_DIVISION
} operation_t;

} // namespace arbos

#endif // ARBOS_TYPES_HPP
