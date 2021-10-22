/*******************************************************************************
 *
 * \file
 * \brief Example of memory location management
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
#include <ikos/core/semantic/memory_location.hpp>
#include <ikos/core/support/assert.hpp>

namespace ikos {
namespace core {
namespace example {

/// \brief Simple memory location factory based on strings
///
/// This is an example of a memory location factory.
///
/// Memory locations provided by this factory have a name and an index.
///
/// This factory can be used for any scalar abstract domain, memory abstract
/// domain, lifetime abstract domain, etc.
class MemoryFactory {
public:
  class MemoryLocation {
  public:
    friend class MemoryFactory;

  private:
    std::string _name;
    Index _id;

  private:
    /// \brief Private constructor
    MemoryLocation(std::string name, Index id)
        : _name(std::move(name)), _id(id) {}

  public:
    /// \brief No default constructor
    MemoryLocation() = delete;

    /// \brief No copy constructor
    MemoryLocation(const MemoryLocation&) = delete;

    /// \brief Move constructor
    MemoryLocation(MemoryLocation&&) = default;

    /// \brief No copy assignment operator
    MemoryLocation& operator=(const MemoryLocation&) = delete;

    /// \brief Move assignment operator
    MemoryLocation& operator=(MemoryLocation&&) = default;

    /// \brief Destructor
    ~MemoryLocation() = default;

    /// \brief Return the name of the memory location
    const std::string& name() const { return this->_name; }

    /// \brief Return the unique index of the memory location
    Index index() const { return this->_id; }

  }; // end class MemoryLocation

public:
  using MemoryLocationRef = const MemoryLocation*;

private:
  using Map = std::unordered_map< std::string, MemoryLocation >;

private:
  Index _next_id = 1;
  Map _map;

public:
  /// \brief Create a memory factory
  MemoryFactory() = default;

  /// \brief Create a memory factory, starting with the given index
  explicit MemoryFactory(Index start_id) : _next_id(start_id) {}

  /// \brief No copy constructor
  MemoryFactory(const MemoryFactory&) = delete;

  /// \brief No move constructor
  MemoryFactory(MemoryFactory&&) = delete;

  /// \brief No copy assignment operator
  MemoryFactory& operator=(const MemoryFactory&) = delete;

  /// \brief No move assignment operator
  MemoryFactory& operator=(MemoryFactory&&) = delete;

  /// \brief Destructor
  ~MemoryFactory() = default;

  /// \brief Get or create a memory location with the given name
  MemoryLocationRef get(const std::string& name) {
    // This is sound because references are kept valid when using
    // std::unordered_map::emplace()
    auto it = this->_map.find(name);
    if (it != this->_map.end()) {
      return &(it->second);
    } else {
      auto res =
          this->_map.emplace(name, MemoryLocation(name, this->_next_id++));
      ikos_assert(res.second);
      return &(res.first->second);
    }
  }

}; // end class MemoryFactory

/// \brief Write a memory location on a stream
inline std::ostream& operator<<(std::ostream& o,
                                MemoryFactory::MemoryLocationRef m) {
  o << m->name();
  return o;
}

} // end namespace example
} // end namespace core
} // end namespace ikos

namespace ikos {
namespace core {

/// \brief Implement IndexableTraits for
/// example::MemoryFactory::MemoryLocationRef
template <>
struct IndexableTraits< example::MemoryFactory::MemoryLocationRef > {
  static Index index(example::MemoryFactory::MemoryLocationRef m) {
    return m->index();
  }
};

/// \brief Implement DumpableTraits for
/// example::MemoryFactory::MemoryLocationRef
template <>
struct DumpableTraits< example::MemoryFactory::MemoryLocationRef > {
  static void dump(std::ostream& o,
                   example::MemoryFactory::MemoryLocationRef m) {
    o << m->name();
  }
};

} // end namespace core
} // end namespace ikos
