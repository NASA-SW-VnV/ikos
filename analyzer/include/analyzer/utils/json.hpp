/*******************************************************************************
 *
 * Data structures for the manipulation of JSON objects.
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2016-2017 United States Government as represented by the
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

#ifndef ANALYZER_JSON_HPP
#define ANALYZER_JSON_HPP

#include <sstream>
#include <iomanip>

#include <ikos/number/z_number.hpp>

namespace analyzer {

class json_node {
public:
  virtual void write(std::ostream& o) const = 0;

  virtual ~json_node() {}

}; // end class json_node

inline const json_node& to_json(const json_node& n) {
  return n;
}

class json_integer : public json_node {
private:
  ikos::z_number _n;

public:
  explicit json_integer(ikos::z_number n) : _n(n) {}

  void write(std::ostream& o) const { o << _n; }

}; // end class json_integer

inline json_integer to_json(int n) {
  return json_integer(ikos::z_number(n));
}
inline json_integer to_json(ikos::z_number n) {
  return json_integer(n);
}

class json_float : public json_node {
private:
  double _d;

public:
  explicit json_float(double d) : _d(d) {}

  void write(std::ostream& o) const { o << _d; }

}; // end class json_float

inline json_float to_json(float f) {
  return json_float(f);
}
inline json_float to_json(double d) {
  return json_float(d);
}

class json_bool : public json_node {
private:
  bool _b;

public:
  explicit json_bool(bool b) : _b(b) {}

  void write(std::ostream& o) const {
    if (_b) {
      o << "true";
    } else {
      o << "false";
    }
  }

}; // end class json_bool

inline json_bool to_json(bool b) {
  return json_bool(b);
}

class json_string : public json_node {
private:
  std::string _s;

public:
  explicit json_string(const std::string& s) : _s(s) {}

  void write(std::ostream& o) const {
    char previous_fill = o.fill();
    o << '"';
    for (const unsigned char c : _s) {
      if (c == '"' || c == '\\') {
        o << '\\' << c;
      } else if (c >= 0x20 && c <= 0x7E) { // ascii
        o << c;
      } else if (c == '\b') {
        o << "\\b";
      } else if (c == '\t') {
        o << "\\t";
      } else if (c == '\n') {
        o << "\\n";
      } else if (c == '\f') {
        o << "\\f";
      } else if (c == '\r') {
        o << "\\r";
      } else {
        o << "\\u" << std::hex << std::setfill('0') << std::setw(4)
          << static_cast< unsigned int >(c) << std::dec;
      }
    }
    o << '"';
    o.fill(previous_fill);
  }

}; // end class json_string

inline json_string to_json(const std::string& s) {
  return json_string(s);
}
inline json_string to_json(const char* s) {
  return json_string(s);
}

class json_list : public json_node {
private:
  // because we only need to define write(std::ostream&), there is no need to
  // keep all elements in the list
  std::stringstream _buf;
  bool _empty;

public:
  json_list() : _buf(std::ios_base::out | std::ios_base::ate), _empty(true) {}

  json_list(const json_list& o)
      : _buf(o._buf.str(), std::ios_base::out | std::ios_base::ate),
        _empty(o._empty) {}

  template < typename... Args >
  json_list(const Args&... args)
      : _buf(std::ios_base::out | std::ios_base::ate), _empty(true) {
    recursive_add(args...);
  }

  json_list& operator=(const json_list& o) {
    _buf.str(o._buf.str());
    _buf.clear();
    _empty = o._empty;
    return *this;
  }

  void clear() {
    _buf.str("");
    _buf.clear();
    _empty = true;
  }

  bool empty() const { return _empty; }

  template < typename T >
  void add(const T& v) {
    if (!_empty) {
      _buf << ",";
    }
    to_json(v).write(_buf);
    _empty = false;
  }

  void write(std::ostream& o) const { o << "[" << _buf.str() << "]"; }

private:
  // implementation details for json_list(const Args&... args)

  void recursive_add() {}

  template < typename T, typename... Args >
  void recursive_add(const T& v, const Args&... args) {
    add(v);
    recursive_add(args...);
  }

}; // end class json_list

template < typename Iterator >
inline json_list to_json(Iterator begin, Iterator end) {
  json_list l;
  for (auto it = begin; it != end; ++it) {
    l.add(*it);
  }
  return l;
}

class json_dict : public json_node {
private:
  // because we only need to define write(std::ostream&), there is no need to
  // keep all elements in the dict
  std::stringstream _buf;
  bool _empty;

private:
  // only for json_dict(std::initializer_list< binding >)
  struct binding {
    std::stringstream _buf;

    template < typename T >
    binding(const std::string& key, const T& value)
        : _buf(std::ios_base::out | std::ios_base::ate) {
      json_string(key).write(_buf);
      _buf << ":";
      to_json(value).write(_buf);
    }

    binding(const binding& o)
        : _buf(o._buf.str(), std::ios_base::out | std::ios_base::ate) {}

    void write(std::ostream& o) const { o << _buf.str(); }
  };

public:
  json_dict() : _buf(std::ios_base::out | std::ios_base::ate), _empty(true) {}

  json_dict(const json_dict& o)
      : _buf(o._buf.str(), std::ios_base::out | std::ios_base::ate),
        _empty(o._empty) {}

  json_dict(std::initializer_list< binding > l)
      : _buf(std::ios_base::out | std::ios_base::ate), _empty(true) {
    for (auto it = l.begin(); it != l.end(); ++it) {
      if (!_empty) {
        _buf << ",";
      }
      it->write(_buf);
      _empty = false;
    }
  }

  json_dict& operator=(const json_dict& o) {
    _buf.str(o._buf.str());
    _buf.clear();
    _empty = o._empty;
    return *this;
  }

  void clear() {
    _buf.str("");
    _buf.clear();
    _empty = true;
  }

  bool empty() const { return _empty; }

  template < typename T >
  void put(const std::string& key, const T& value) {
    if (!_empty) {
      _buf << ",";
    }
    json_string(key).write(_buf);
    _buf << ":";
    to_json(value).write(_buf);
    _empty = false;
  }

  void write(std::ostream& o) const { o << "{" << _buf.str() << "}"; }

}; // end class json_dict

inline std::ostream& operator<<(std::ostream& o, const json_node& n) {
  n.write(o);
  return o;
}

} // end namespace analyzer

#endif // ANALYZER_JSON_HPP
