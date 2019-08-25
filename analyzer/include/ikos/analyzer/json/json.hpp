/*******************************************************************************
 *
 * \file
 * \brief Data structures for the manipulation of JSON objects
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2016-2019 United States Government as represented by the
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

#include <string>

#include <ikos/analyzer/support/number.hpp>

namespace ikos {
namespace analyzer {

/// \brief Base class for JSON objects
class JsonNode {
public:
  /// \brief Constructor
  JsonNode() = default;

  /// \brief Copy constructor
  JsonNode(const JsonNode&) noexcept = default;

  /// \brief Move constructor
  JsonNode(JsonNode&&) noexcept = default;

  /// \brief Copy assignment operator
  JsonNode& operator=(const JsonNode&) noexcept = default;

  /// \brief Move assignment operator
  JsonNode& operator=(JsonNode&&) noexcept = default;

  /// \brief Return the string representation
  virtual std::string str() const = 0;

  /// \brief Destructor
  virtual ~JsonNode();

}; // end class JsonNode

/// \brief Convert basic types to JsonNode
inline const JsonNode& to_json(const JsonNode& n) {
  return n;
}

/// \brief A JSON integer
class JsonInteger final : public JsonNode {
private:
  ZNumber _n;

public:
  /// \brief Constructor
  explicit JsonInteger(ZNumber n) : _n(std::move(n)) {}

  /// \brief Return the string representation
  std::string str() const override;

}; // end class JsonInteger

/// \brief Convert integers to JsonInteger
template < typename T,
           class = std::enable_if_t< core::IsSupportedIntegral< T >::value > >
inline JsonInteger to_json(T n) {
  return JsonInteger(ZNumber(n));
}

/// \brief Convert integers to JsonInteger
inline JsonInteger to_json(ZNumber n) {
  return JsonInteger(std::move(n));
}

/// \brief Convert integers to JsonInteger
inline JsonInteger to_json(const MachineInt& n) {
  return JsonInteger(n.to_z_number());
}

/// \brief A JSON floating point
class JsonFloat final : public JsonNode {
private:
  double _d;

public:
  /// \brief Constructor
  explicit JsonFloat(double d) : _d(d) {}

  /// \brief Return the string representation
  std::string str() const override;

}; // end class JsonFloat

/// \brief Convert floating points to JsonFloat
inline JsonFloat to_json(float f) {
  return JsonFloat(static_cast< double >(f));
}

/// \brief Convert floating points to JsonFloat
inline JsonFloat to_json(double d) {
  return JsonFloat(d);
}

/// \brief A JSON boolean
class JsonBool final : public JsonNode {
private:
  bool _b;

public:
  /// \brief Constructor
  explicit JsonBool(bool b) : _b(b) {}

  /// \brief Return the string representation
  std::string str() const override;

}; // end class JsonBool

/// \brief Convert booleans to JsonBool
inline JsonBool to_json(bool b) {
  return JsonBool(b);
}

/// \brief A JSON string
class JsonString final : public JsonNode {
private:
  std::string _s;

public:
  /// \brief Constructor
  explicit JsonString(std::string s) : _s(std::move(s)) {}

  /// \brief Return the string representation
  std::string str() const override;

}; // end class JsonString

/// \brief Convert strings to JsonString
inline JsonString to_json(std::string s) {
  return JsonString(std::move(s));
}

/// \brief Convert strings to JsonString
inline JsonString to_json(const char* s) {
  return JsonString(s);
}

/// \brief A JSON list
class JsonList final : public JsonNode {
private:
  // No need to keep all elements of the list to implement str()

  /// \brief String representation
  std::string _buf;

public:
  /// \brief Create an empty list
  JsonList() = default;

  /// \brief Copy constructor
  JsonList(const JsonList&) = default;

  /// \brief Create a list with the given parameters
  template < typename... Args >
  explicit JsonList(const Args&... args) {
    recursive_add(args...);
  }

  /// \brief Move constructor
  JsonList(JsonList&&) = default;

  /// \brief Copy assignment operator
  JsonList& operator=(const JsonList&) = default;

  /// \brief Move assignment operator
  JsonList& operator=(JsonList&&) = default;

  /// \brief Destructor
  ~JsonList() override = default;

  /// \brief Clear the list
  void clear() { this->_buf.clear(); }

  /// \brief Return true if the list is empty
  bool empty() const { return this->_buf.empty(); }

  /// \brief Add an element to the list
  template < typename T >
  void add(const T& v) {
    if (!this->_buf.empty()) {
      this->_buf.push_back(',');
    }
    this->_buf.append(to_json(v).str());
  }

  /// \brief Return the string representation
  std::string str() const override;

private:
  // Implementation details for JsonList(const Args&... args)

  void recursive_add() {}

  template < typename T, typename... Args >
  void recursive_add(const T& v, const Args&... args) {
    add(v);
    recursive_add(args...);
  }

}; // end class JsonList

/// \brief Create a JsonList with the given range
template < typename Iterator >
inline JsonList to_json(Iterator begin, Iterator end) {
  JsonList l;
  for (auto it = begin; it != end; ++it) {
    l.add(*it);
  }
  return l;
}

/// \brief A JSON dictionary
class JsonDict final : public JsonNode {
private:
  // No need to keep all elements of the dict to implement str()

  /// \brief String representation
  std::string _buf;

private:
  // Helper for JsonDict(std::initializer_list< Binding >)
  class Binding {
  private:
    std::string _buf;

  public:
    /// \brief Constructor
    template < typename T >
    Binding(std::string key, const T& value) {
      this->_buf.append(JsonString(std::move(key)).str());
      this->_buf.push_back(':');
      this->_buf.append(to_json(value).str());
    }

    /// \brief Copy constructor
    Binding(const Binding&) = default;

    /// \brief No move constructor
    Binding(Binding&&) = delete;

    /// \brief No copy assignment operator
    Binding& operator=(const Binding&) = delete;

    /// \brief No move assignment operator
    Binding& operator=(Binding&&) = delete;

    /// \brief Destructor
    ~Binding() = default;

    const std::string& str() const { return this->_buf; }
  };

public:
  /// \brief Create an empty dictionary
  JsonDict() = default;

  /// \brief Copy constructor
  JsonDict(const JsonDict&) = default;

  /// \brief Create a dictionary with the given pairs
  JsonDict(std::initializer_list< Binding > l) {
    for (const auto& binding : l) {
      if (!this->_buf.empty()) {
        this->_buf.push_back(',');
      }
      this->_buf.append(binding.str());
    }
  }

  /// \brief Move constructor
  JsonDict(JsonDict&&) = default;

  /// \brief Copy assignment operator
  JsonDict& operator=(const JsonDict&) = default;

  /// \brief Move assignment operator
  JsonDict& operator=(JsonDict&&) = default;

  /// \brief Destructor
  ~JsonDict() override = default;

  /// \brief Clear the dictionary
  void clear() { this->_buf.clear(); }

  /// \brief Return true if the dictionary is empty
  bool empty() const { return this->_buf.empty(); }

  /// \brief Add a (key, value) pair in the dictionary
  template < typename T >
  void put(std::string key, const T& value) {
    if (!this->_buf.empty()) {
      this->_buf.push_back(',');
    }
    this->_buf.append(JsonString(std::move(key)).str());
    this->_buf.push_back(':');
    this->_buf.append(to_json(value).str());
  }

  /// \brief Return the string representation
  std::string str() const override;

}; // end class JsonDict

/// \brief Write a JSON node on a stream
inline std::ostream& operator<<(std::ostream& o, const JsonNode& n) {
  o << n.str();
  return o;
}

} // end namespace analyzer
} // end namespace ikos
