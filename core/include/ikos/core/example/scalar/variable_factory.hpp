/*******************************************************************************
 *
 * \file
 * \brief Example of scalar variable management
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2019 United States Government as represented by the
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
#include <unordered_map>

#include <ikos/core/semantic/dumpable.hpp>
#include <ikos/core/semantic/indexable.hpp>
#include <ikos/core/semantic/machine_int/variable.hpp>
#include <ikos/core/semantic/scalar/variable.hpp>
#include <ikos/core/semantic/variable.hpp>
#include <ikos/core/support/assert.hpp>

namespace ikos {
namespace core {
namespace example {
namespace scalar {

/// \brief Simple scalar variable factory based on strings
///
/// This is an example of a variable factory.
///
/// Variables provided by this factory have a name and an index.
///
/// This factory can be used for any scalar abstract domain or memory abstract
/// domain.
class VariableFactory {
public:
  enum VariableKind {
    IntVariableKind,
    FloatVariableKind,
    PointerVariableKind,
    DynamicVariableKind,
  };

  class Variable {
  private:
    /// \brief Variable name
    std::string _name;

    /// \brief Variable index
    Index _id;

    /// \brief Variable kind
    VariableKind _kind;

    /// \brief Variable bit width (if any)
    uint64_t _bit_width;

    /// \brief Variable signedness (if any)
    Signedness _sign;

    /// \brief Offset variable (if any)
    Variable* _offset_var;

  private:
    /// \brief Private constructor
    Variable(std::string name,
             Index id,
             VariableKind kind,
             uint64_t bit_width,
             Signedness sign,
             Variable* offset_var)
        : _name(std::move(name)),
          _id(id),
          _kind(kind),
          _bit_width(bit_width),
          _sign(sign),
          _offset_var(offset_var) {}

  public:
    static Variable make_int(std::string name,
                             Index id,
                             uint64_t bit_width,
                             Signedness sign) {
      return {std::move(name), id, IntVariableKind, bit_width, sign, nullptr};
    }

    static Variable make_float(std::string name, Index id) {
      return {std::move(name), id, FloatVariableKind, 0, Signed, nullptr};
    }

    static Variable make_pointer(std::string name,
                                 Index id,
                                 Variable* offset_var) {
      return {std::move(name), id, PointerVariableKind, 0, Signed, offset_var};
    }

    static Variable make_dynamic(std::string name,
                                 Index id,
                                 uint64_t bit_width,
                                 Signedness sign,
                                 Variable* offset_var) {
      return {std::move(name),
              id,
              DynamicVariableKind,
              bit_width,
              sign,
              offset_var};
    }

    /// \brief No default constructor
    Variable() = delete;

    /// \brief No copy constructor
    Variable(const Variable&) = delete;

    /// \brief Move constructor
    Variable(Variable&&) = default;

    /// \brief No copy assignment operator
    Variable& operator=(const Variable&) = delete;

    /// \brief Move assignment operator
    Variable& operator=(Variable&&) = default;

    /// \brief Destructor
    ~Variable() = default;

    /// \brief Return the name of the variable
    const std::string& name() const { return this->_name; }

    /// \brief Return the unique index of the variable
    Index index() const { return this->_id; }

    /// \brief Return the kind of the variable
    VariableKind kind() const { return this->_kind; }

    /// \brief Return the bit width of the variable
    uint64_t bit_width() const {
      ikos_assert(this->_kind == IntVariableKind ||
                  this->_kind == DynamicVariableKind);
      return this->_bit_width;
    }

    /// \brief Return the signedness of the variable
    Signedness sign() const {
      ikos_assert(this->_kind == IntVariableKind ||
                  this->_kind == DynamicVariableKind);
      return this->_sign;
    }

    /// \brief Return the offset variable of the variable
    Variable* offset_var() const {
      ikos_assert(this->_kind == PointerVariableKind ||
                  this->_kind == DynamicVariableKind);
      return this->_offset_var;
    }

  }; // end class Variable

public:
  using VariableRef = const Variable*;

private:
  using Map = std::unordered_map< std::string, Variable >;

private:
  Index _next_id = 1;
  Map _map;

public:
  /// \brief Create a variable factory
  VariableFactory() = default;

  /// \brief Create a variable factory, starting with the given index
  explicit VariableFactory(Index start_id) : _next_id(start_id) {}

  /// \brief No copy constructor
  VariableFactory(const VariableFactory&) = delete;

  /// \brief No move constructor
  VariableFactory(VariableFactory&&) = delete;

  /// \brief No copy assignment operator
  VariableFactory& operator=(const VariableFactory&) = delete;

  /// \brief No move assignment operator
  VariableFactory& operator=(VariableFactory&&) = delete;

  /// \brief Destructor
  ~VariableFactory() = default;

  /// \brief Get or create a machine integer variable
  VariableRef get_int(const std::string& name,
                      uint64_t bit_width,
                      Signedness sign) {
    // This is sound because references are kept valid when using
    // std::unordered_map::emplace()

    auto it = this->_map.find(name);
    if (it != this->_map.end()) {
      return &(it->second);
    }

    auto res = this->_map.emplace(name,
                                  Variable::make_int(name,
                                                     this->_next_id++,
                                                     bit_width,
                                                     sign));
    ikos_assert(res.second);
    return &(res.first->second);
  }

  /// \brief Get or create a floating point variable
  VariableRef get_float(const std::string& name) {
    // This is sound because references are kept valid when using
    // std::unordered_map::emplace()

    auto it = this->_map.find(name);
    if (it != this->_map.end()) {
      return &(it->second);
    }

    auto res =
        this->_map.emplace(name, Variable::make_float(name, this->_next_id++));
    ikos_assert(res.second);
    return &(res.first->second);
  }

  /// \brief Get or create a pointer variable
  VariableRef get_pointer(const std::string& name,
                          uint64_t offset_bit_width,
                          Signedness offset_sign) {
    // This is sound because references are kept valid when using
    // std::unordered_map::emplace()

    auto it = this->_map.find(name);
    if (it != this->_map.end()) {
      return &(it->second);
    }

    auto res = this->_map.emplace(name + ".offset",
                                  Variable::make_int(name + ".offset",
                                                     this->_next_id++,
                                                     offset_bit_width,
                                                     offset_sign));
    ikos_assert(res.second);
    Variable* offset_var = &(res.first->second);

    res = this->_map.emplace(name,
                             Variable::make_pointer(name,
                                                    this->_next_id++,
                                                    offset_var));
    ikos_assert(res.second);
    return &(res.first->second);
  }

  /// \brief Get or create a dynamically typed variable
  VariableRef get_dynamic(const std::string& name,
                          uint64_t bit_width,
                          Signedness sign,
                          uint64_t offset_bit_width,
                          Signedness offset_sign) {
    // This is sound because references are kept valid when using
    // std::unordered_map::emplace()

    auto it = this->_map.find(name);
    if (it != this->_map.end()) {
      return &(it->second);
    }

    auto res = this->_map.emplace(name + ".offset",
                                  Variable::make_int(name + ".offset",
                                                     this->_next_id++,
                                                     offset_bit_width,
                                                     offset_sign));
    ikos_assert(res.second);
    Variable* offset_var = &(res.first->second);

    res = this->_map.emplace(name,
                             Variable::make_dynamic(name,
                                                    this->_next_id++,
                                                    bit_width,
                                                    sign,
                                                    offset_var));
    ikos_assert(res.second);
    return &(res.first->second);
  }

}; // end class VariableFactory

/// \brief Write a variable on a stream
inline std::ostream& operator<<(std::ostream& o,
                                VariableFactory::VariableRef var) {
  o << var->name();
  return o;
}

} // end namespace scalar
} // end namespace example
} // end namespace core
} // end namespace ikos

namespace ikos {
namespace core {

/// \brief Implement IndexableTraits for
/// example::scalar::VariableFactory::VariableRef
template <>
struct IndexableTraits< example::scalar::VariableFactory::VariableRef > {
  static Index index(example::scalar::VariableFactory::VariableRef var) {
    return var->index();
  }
};

/// \brief Implement DumpableTraits for
/// example::scalar::VariableFactory::VariableRef
template <>
struct DumpableTraits< example::scalar::VariableFactory::VariableRef > {
  static void dump(std::ostream& o,
                   example::scalar::VariableFactory::VariableRef var) {
    o << var->name();
  }
};

namespace machine_int {

/// \brief Implement VariableTraits for
/// example::scalar::VariableFactory::VariableRef
template <>
struct VariableTraits< example::scalar::VariableFactory::VariableRef > {
  static uint64_t bit_width(example::scalar::VariableFactory::VariableRef var) {
    return var->bit_width();
  }

  static Signedness sign(example::scalar::VariableFactory::VariableRef var) {
    return var->sign();
  }
};

} // end namespace machine_int

namespace scalar {

/// \brief Implement VariableTraits for
/// example::scalar::VariableFactory::VariableRef
template <>
struct VariableTraits< example::scalar::VariableFactory::VariableRef > {
  static bool is_int(example::scalar::VariableFactory::VariableRef var) {
    return var->kind() == example::scalar::VariableFactory::IntVariableKind;
  }

  static bool is_float(example::scalar::VariableFactory::VariableRef var) {
    return var->kind() == example::scalar::VariableFactory::FloatVariableKind;
  }

  static bool is_pointer(example::scalar::VariableFactory::VariableRef var) {
    return var->kind() == example::scalar::VariableFactory::PointerVariableKind;
  }

  static bool is_dynamic(example::scalar::VariableFactory::VariableRef var) {
    return var->kind() == example::scalar::VariableFactory::DynamicVariableKind;
  }

  static example::scalar::VariableFactory::VariableRef offset_var(
      example::scalar::VariableFactory::VariableRef var) {
    return var->offset_var();
  }
};

} // end namespace scalar

} // end namespace core
} // end namespace ikos
