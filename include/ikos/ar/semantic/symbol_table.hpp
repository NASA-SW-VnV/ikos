/*******************************************************************************
 *
 * \file
 * \brief Symbol table definition
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

#include <memory>
#include <string>
#include <unordered_map>

#include <boost/iterator/transform_iterator.hpp>

#include <ikos/ar/support/assert.hpp>
#include <ikos/ar/support/iterator.hpp>

namespace ikos {
namespace ar {

/// \brief Symbol table
///
/// Represents a list of symbols.
template < typename T >
class SymbolTable {
private:
  // Map from names to symbols
  std::unordered_map< std::string, std::unique_ptr< T > > _map;

public:
  /// \brief Iterator over the symbols
  using Iterator = boost::transform_iterator<
      MapExposeRawPtr< std::string, T >,
      typename std::unordered_map< std::string,
                                   std::unique_ptr< T > >::const_iterator >;

public:
  /// \brief Default constructor
  explicit SymbolTable() = default;

  /// \brief Copy constructor
  SymbolTable(const SymbolTable&) = default;

  /// \brief Move constructor
  SymbolTable(SymbolTable&&) = default;

  /// \brief Copy assignment operator
  SymbolTable& operator=(const SymbolTable&) = default;

  /// \brief Move assignment operator
  SymbolTable& operator=(SymbolTable&&) = default;

  /// \brief Destructor
  ~SymbolTable() = default;

  /// \brief Add a symbol
  ///
  /// This is undefined behavior if the symbol name is already taken.
  void add(std::unique_ptr< T > symbol) {
    ikos_assert_msg(!symbol->name().empty(), "name is empty");
    ikos_assert_msg(!this->contains(symbol->name()), "name already taken");

    this->_map.emplace(symbol->name(), std::move(symbol));
  }

  /// \brief Rename the given symbol
  ///
  /// \param symbol The symbol
  /// \param prev_name The previous name
  /// \param new_name The new name
  ///
  /// This is undefined behavior if the new symbol name is already taken.
  void rename(T* symbol,
              const std::string& prev_name,
              const std::string& new_name) {
    ikos_assert_msg(!prev_name.empty(), "name is empty");
    ikos_assert_msg(this->find(prev_name) == symbol, "symbol not registered");
    ikos_assert_msg(!new_name.empty(), "name is empty");
    ikos_assert_msg(!this->contains(new_name), "name already taken");
    ikos_ignore(symbol);

    auto it = this->_map.find(prev_name);
    std::unique_ptr< T > symbol_ptr = std::move(it->second);
    this->_map.erase(it);
    this->_map.emplace(new_name, std::move(symbol_ptr));
  }

  /// \brief Return true if the table is empty
  bool empty() const { return this->_map.empty(); }

  /// \brief Return the size of the table
  std::size_t size() const { return this->_map.size(); }

  /// \brief Begin iterator over the symbols
  Iterator begin() const {
    return boost::make_transform_iterator(this->_map.cbegin(),
                                          MapExposeRawPtr< std::string, T >());
  }

  /// \brief End iterator over the symbols
  Iterator end() const {
    return boost::make_transform_iterator(this->_map.cend(),
                                          MapExposeRawPtr< std::string, T >());
  }

  /// \brief Return the symbol with the given name, or nullptr
  T* find(const std::string& name) const {
    ikos_assert_msg(!name.empty(), "name is empty");

    auto it = this->_map.find(name);
    if (it != this->_map.end()) {
      return it->second.get();
    } else {
      return nullptr;
    }
  }

  /// \brief Return true if the table contains a symbol with the given name
  bool contains(const std::string& name) const {
    ikos_assert_msg(!name.empty(), "name is empty");

    return this->_map.find(name) != this->_map.end();
  }

}; // end class SymbolTable

} // end namespace ar
} // end namespace ikos
