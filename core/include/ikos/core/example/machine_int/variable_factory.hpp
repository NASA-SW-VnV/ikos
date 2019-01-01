/*******************************************************************************
 *
 * \file
 * \brief Example of machine integer variable management
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

#include <string>
#include <unordered_map>

#include <ikos/core/semantic/dumpable.hpp>
#include <ikos/core/semantic/indexable.hpp>
#include <ikos/core/semantic/machine_int/variable.hpp>
#include <ikos/core/semantic/variable.hpp>

namespace ikos {
namespace core {
namespace example {
namespace machine_int {

/// \brief Simple machine integer variable factory based on strings
///
/// This is an example of a variable factory.
///
/// Variables provided by this factory have a name, an index, a bit-width and a
/// signedness.
///
/// This factory can be used for any machine integer abstract domain (namespace
/// `machine_int`).
class VariableFactory {
public:
  class Variable {
  public:
    friend class VariableFactory;

  private:
    std::string _name;
    unsigned _bit_width;
    Signedness _sign;
    Index _id;

  private:
    struct PrivateTag {};

  public:
    /// \brief Private constructor
    Variable(std::string name,
             unsigned bit_width,
             Signedness sign,
             Index id,
             PrivateTag)
        : _name(std::move(name)), _bit_width(bit_width), _sign(sign), _id(id) {}

    /// \brief Deleted default constructor, copy constructors, etc.
    Variable() = delete;
    Variable(const Variable&) = delete;
    Variable(Variable&&) = delete;
    Variable& operator=(const Variable&) = delete;
    Variable& operator=(Variable&&) = delete;

    /// \brief Destructor
    ~Variable() = default;

    /// \brief Return the name of the variable
    const std::string& name() const { return this->_name; }

    /// \brief Return the bit-width of the variable
    unsigned bit_width() const { return this->_bit_width; }

    /// \brief Return the sign of the variable
    Signedness sign() const { return this->_sign; }

    /// \brief Return the unique index of the variable
    Index index() const { return this->_id; }

  }; // end class Variable

public:
  using VariableRef = std::shared_ptr< const Variable >;

private:
  using Map = std::unordered_map< std::string, VariableRef >;

private:
  Index _next_id = 1;
  Map _map;

public:
  /// \brief Create a variable factory
  VariableFactory() = default;

  /// \brief Create a variable factory, starting with the given index
  explicit VariableFactory(Index start_id) : _next_id(start_id) {}

  /// \brief Delete copy constructor, copy assignment constructor, etc.
  VariableFactory(const VariableFactory&) = delete;
  VariableFactory(VariableFactory&&) = delete;
  VariableFactory& operator=(const VariableFactory&) = delete;
  VariableFactory& operator=(VariableFactory&&) = delete;

  /// \brief Destructor
  ~VariableFactory() = default;

  /// \brief Get the variable reference for the given string, bit-width and sign
  VariableRef get(const std::string& name,
                  unsigned bit_width,
                  Signedness sign) {
    auto it = this->_map.find(name);
    if (it != this->_map.end()) {
      return it->second;
    } else {
      auto var = std::make_shared< const Variable >(name,
                                                    bit_width,
                                                    sign,
                                                    this->_next_id++,
                                                    Variable::PrivateTag{});
      this->_map.emplace(name, var);
      return var;
    }
  }

}; // end class VariableFactory

/// \brief Write a variable on a stream
inline std::ostream& operator<<(std::ostream& o,
                                const VariableFactory::VariableRef& var) {
  o << var->name();
  return o;
}

} // end namespace machine_int
} // end namespace example
} // end namespace core
} // end namespace ikos

namespace ikos {
namespace core {

/// \brief Implement IndexableTraits for
/// example::machine_int::VariableFactory::VariableRef
template <>
struct IndexableTraits< example::machine_int::VariableFactory::VariableRef > {
  static Index index(
      const example::machine_int::VariableFactory::VariableRef& var) {
    return var->index();
  }
};

/// \brief Implement DumpableTraits for
/// example::machine_int::VariableFactory::VariableRef
template <>
struct DumpableTraits< example::machine_int::VariableFactory::VariableRef > {
  static void dump(
      std::ostream& o,
      const example::machine_int::VariableFactory::VariableRef& var) {
    o << var->name();
  }
};

namespace machine_int {

/// \brief Implement VariableTraits for
/// example::machine_int::VariableFactory::VariableRef
template <>
struct VariableTraits< example::machine_int::VariableFactory::VariableRef > {
  static unsigned bit_width(
      const example::machine_int::VariableFactory::VariableRef& var) {
    return var->bit_width();
  }

  static Signedness sign(
      const example::machine_int::VariableFactory::VariableRef& var) {
    return var->sign();
  }
};

} // end namespace machine_int

} // end namespace core
} // end namespace ikos
