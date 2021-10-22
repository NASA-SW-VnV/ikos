/*******************************************************************************
 *
 * \file
 * \brief Bundle definition
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2017-2019 United States Government as represented by the
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

#include <ikos/ar/semantic/context.hpp>
#include <ikos/ar/semantic/data_layout.hpp>
#include <ikos/ar/semantic/intrinsic.hpp>
#include <ikos/ar/semantic/symbol_table.hpp>
#include <ikos/ar/support/string_ref.hpp>
#include <ikos/ar/support/traceable.hpp>

namespace ikos {
namespace ar {

// forward declaration
class GlobalVariable;
class Function;

/// \brief AR bundle
///
/// A bundle is the main container representing a whole program.
///
/// It contains a list of global variables and functions.
class Bundle : public Traceable {
private:
  // AR Context
  Context& _context;

  // Data Layout
  std::unique_ptr< DataLayout > _data_layout;

  // Target triple
  std::string _target_triple;

  // List of global variables
  SymbolTable< GlobalVariable > _globals;

  // List of functions
  SymbolTable< Function > _functions;

public:
  /// \brief Iterator over a list of global variables
  using GlobalVariableIterator = SymbolTable< GlobalVariable >::Iterator;

  /// \brief Iterator over a list of functions
  using FunctionIterator = SymbolTable< Function >::Iterator;

private:
  /// \brief Private constructor
  Bundle(Context& ctx,
         std::unique_ptr< DataLayout > data_layout,
         std::string triple);

public:
  /// \brief No copy constructor
  Bundle(const Bundle&) = delete;

  /// \brief No move constructor
  Bundle(Bundle&&) = delete;

  /// \brief No copy assignment operator
  Bundle& operator=(const Bundle&) = delete;

  /// \brief No move assignment operator
  Bundle& operator=(Bundle&&) = delete;

  /// \brief Destructor
  ~Bundle();

  /// \brief Static constructor
  static Bundle* create(Context& ctx,
                        std::unique_ptr< DataLayout > data_layout,
                        std::string triple);

  /// \brief Get the parent context
  Context& context() const { return this->_context; }

  /// \brief Get the data layout
  DataLayout& data_layout() { return *this->_data_layout; }

  /// \brief Get the data layout
  const DataLayout& data_layout() const { return *this->_data_layout; }

  /// \brief Get the target triple
  const std::string& target_triple() const { return this->_target_triple; }

  /// \brief Begin iterator over the list of global variables
  GlobalVariableIterator global_begin() const { return this->_globals.begin(); }

  /// \brief End iterator over the list of global variables
  GlobalVariableIterator global_end() const { return this->_globals.end(); }

  /// \brief Return the number of global variables
  std::size_t num_globals() const { return this->_globals.size(); }

  /// \brief Get the global variable with the given name, or nullptr
  GlobalVariable* global_or_null(const std::string& name) const {
    return this->_globals.find(name);
  }

  /// \brief Begin iterator over the list of functions
  FunctionIterator function_begin() const { return this->_functions.begin(); }

  /// \brief End iterator over the list of functions
  FunctionIterator function_end() const { return this->_functions.end(); }

  /// \brief Return the number of functions
  std::size_t num_functions() const { return this->_functions.size(); }

  /// \brief Get or create the intrinsic function with the given id
  ///
  /// Function iterators are invalidated.
  Function* intrinsic_function(Intrinsic::ID id);

  /// \brief Get or create the intrinsic function with the given id and type
  /// parameter
  ///
  /// Function iterators are invalidated.
  Function* intrinsic_function(Intrinsic::ID id, Type* template_ty);

  /// \brief Get the function with the given name, or nullptr
  Function* function_or_null(const std::string& name) const {
    return this->_functions.find(name);
  }

  /// \brief Return true if the given name is available
  bool is_name_available(const std::string& name) const;

  /// \brief Find an available name with the given prefix
  std::string find_available_name(StringRef prefix) const;

private:
  /// \brief Add a global variable in the bundle
  void add_global_variable(std::unique_ptr< GlobalVariable >);

  /// \brief Rename a global variable
  void rename_global_variable(GlobalVariable* gv,
                              const std::string& prev_name,
                              const std::string& new_name);

  /// \brief Add a function in the bundle
  void add_function(std::unique_ptr< Function >);

  /// \brief Rename a function
  void rename_function(Function* fun,
                       const std::string& prev_name,
                       const std::string& new_name);

  // friends
  friend class GlobalVariable;
  friend class Function;

}; // end class Bundle

} // end namespace ar
} // end namespace ikos
