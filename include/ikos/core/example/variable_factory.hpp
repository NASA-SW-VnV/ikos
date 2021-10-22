/*******************************************************************************
 *
 * \file
 * \brief Example of variable management
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

#include <string>
#include <unordered_map>

#include <ikos/core/semantic/dumpable.hpp>
#include <ikos/core/semantic/indexable.hpp>
#include <ikos/core/semantic/variable.hpp>
#include <ikos/core/support/assert.hpp>

namespace ikos {
namespace core {
namespace example {

/// \brief Simple variable factory based on strings
///
/// This is an example of a variable factory.
///
/// Variables provided by this factory have a name and an index.
///
/// This factory can be used for any numerical abstract domain (namespace
/// `numeric`), the nullity domain, the uninitialized domain, etc.
///
/// For a factory to use with a machine integer abstract domain (namespace
/// `machine_int`), see `ikos/core/example/machine_int/variable_factory.hpp`
class VariableFactory {
public:
  class Variable {
  public:
    friend class VariableFactory;

  private:
    std::string _name;
    Index _id;

  private:
    /// \brief Private constructor
    Variable(std::string name, Index id) : _name(std::move(name)), _id(id) {}

  public:
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

  /// \brief Get or create a variable with the given name
  VariableRef get(const std::string& name) {
    // This is sound because references are kept valid when using
    // std::unordered_map::emplace()
    auto it = this->_map.find(name);
    if (it != this->_map.end()) {
      return &(it->second);
    } else {
      auto res = this->_map.emplace(name, Variable(name, this->_next_id++));
      ikos_assert(res.second);
      return &(res.first->second);
    }
  }

}; // end class VariableFactory

/// \brief Write a variable on a stream
inline std::ostream& operator<<(std::ostream& o,
                                VariableFactory::VariableRef var) {
  o << var->name();
  return o;
}

} // end namespace example
} // end namespace core
} // end namespace ikos

namespace ikos {
namespace core {

/// \brief Implement IndexableTraits for example::VariableFactory::VariableRef
template <>
struct IndexableTraits< example::VariableFactory::VariableRef > {
  static Index index(example::VariableFactory::VariableRef var) {
    return var->index();
  }
};

/// \brief Implement DumpableTraits for example::VariableFactory::VariableRef
template <>
struct DumpableTraits< example::VariableFactory::VariableRef > {
  static void dump(std::ostream& o, example::VariableFactory::VariableRef var) {
    o << var->name();
  }
};

} // end namespace core
} // end namespace ikos
