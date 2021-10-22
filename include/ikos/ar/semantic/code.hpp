/*******************************************************************************
 *
 * \file
 * \brief Control flow graph definition
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

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

#include <boost/iterator/transform_iterator.hpp>

#include <ikos/core/semantic/graph.hpp>

#include <ikos/ar/semantic/bundle.hpp>
#include <ikos/ar/semantic/context.hpp>
#include <ikos/ar/semantic/function.hpp>
#include <ikos/ar/semantic/value.hpp>
#include <ikos/ar/support/assert.hpp>
#include <ikos/ar/support/iterator.hpp>
#include <ikos/ar/support/traceable.hpp>

namespace ikos {
namespace ar {

// forward declaration
class Code;
class Statement;

/// \brief Basic block
///
/// A basic block is a container of statements that execute sequentially.
class BasicBlock : public Traceable {
private:
  // List of statements
  std::vector< std::unique_ptr< Statement > > _statements;

  // List of successor basic blocks
  std::vector< BasicBlock* > _successors;

  // List of predecessor basic blocks
  std::vector< BasicBlock* > _predecessors;

  // Parent code
  Code* _parent;

  // Name (optional)
  std::string _name;

public:
  /// \brief Iterator over an ordered list of statements
  using StatementIterator = boost::transform_iterator<
      SeqExposeRawPtr< Statement >,
      std::vector< std::unique_ptr< Statement > >::const_iterator >;

  /// \brief Reverse iterator over an ordered list of statements
  using StatementReverseIterator = boost::transform_iterator<
      SeqExposeRawPtr< Statement >,
      std::vector< std::unique_ptr< Statement > >::const_reverse_iterator >;

  /// \brief Iterator over a list of basic block
  using BasicBlockIterator = std::vector< BasicBlock* >::const_iterator;

private:
  /// \brief Private constructor
  explicit BasicBlock(Code* code);

public:
  /// \brief No copy constructor
  BasicBlock(const BasicBlock&) = delete;

  /// \brief No move constructor
  BasicBlock(BasicBlock&&) = delete;

  /// \brief No copy assignment operator
  BasicBlock& operator=(const BasicBlock&) = delete;

  /// \brief No move assignment operator
  BasicBlock& operator=(BasicBlock&&) = delete;

  /// \brief Destructor
  ~BasicBlock();

  /// \brief Static constructor
  static BasicBlock* create(Code* code);

  /// \brief Get the parent context
  Context& context() const;

  /// \brief Get the parent bundle
  Bundle* bundle() const;

  /// \brief Get the parent code
  Code* code() const { return this->_parent; }

  /// \brief Get the first statement
  Statement* front() const {
    ikos_assert_msg(!this->_statements.empty(), "basic block is empty");
    return this->_statements.front().get();
  }

  /// \brief Get the last statement
  Statement* back() const {
    ikos_assert_msg(!this->_statements.empty(), "basic block is empty");
    return this->_statements.back().get();
  }

  /// \brief Begin iterator over the statements
  StatementIterator begin() const {
    return boost::make_transform_iterator(this->_statements.cbegin(),
                                          SeqExposeRawPtr< Statement >());
  }

  /// \brief End iterator over the statements
  StatementIterator end() const {
    return boost::make_transform_iterator(this->_statements.cend(),
                                          SeqExposeRawPtr< Statement >());
  }

  /// \brief Begin reverse iterator over the statements
  StatementReverseIterator rbegin() const {
    return boost::make_transform_iterator(this->_statements.crbegin(),
                                          SeqExposeRawPtr< Statement >());
  }

  /// \brief End reverse iterator over the statements
  StatementReverseIterator rend() const {
    return boost::make_transform_iterator(this->_statements.crend(),
                                          SeqExposeRawPtr< Statement >());
  }

  /// \brief Return the number of statements
  std::size_t num_statements() const { return this->_statements.size(); }

  /// \brief Return true if the basic block is empty
  bool empty() const { return this->_statements.empty(); }

  /// \brief Insert a statement to the beginning
  ///
  /// Statement iterators are invalidated.
  void push_front(std::unique_ptr< Statement > stmt);

  /// \brief Add a statement to the end
  ///
  /// Statement iterators are invalidated.
  void push_back(std::unique_ptr< Statement > stmt);

  /// \brief Insert a statement before `it`
  ///
  /// Statement iterators are invalidated.
  ///
  /// Returns an iterator on the inserted statement.
  StatementIterator insert_before(StatementIterator it,
                                  std::unique_ptr< Statement > stmt);

  /// \brief Insert a statement after `it`
  ///
  /// Statement iterators are invalidated.
  ///
  /// Returns an iterator on the inserted statement.
  StatementIterator insert_after(StatementIterator it,
                                 std::unique_ptr< Statement > stmt);

  /// \brief Replace the statement at `it`
  ///
  /// Returns the previous statement
  std::unique_ptr< Statement > replace(StatementIterator it,
                                       std::unique_ptr< Statement > stmt);

  /// \brief Remove the statement at `it`
  ///
  /// Statement iterators at or after `it` are invalidated.
  ///
  /// Returns an iterator on the statement following the removed statement. If
  /// `it` refers to the last element, it returns the end iterator.
  StatementIterator remove(StatementIterator it);

  /// \brief Remove the last statement and return it
  ///
  /// The end() statement iterator is invalidated.
  std::unique_ptr< Statement > pop_back();

  /// \brief Remove all statements
  void clear_statements();

  /// \brief Return the number of successors
  std::size_t num_successors() const { return this->_successors.size(); }

  /// \brief Begin iterator over the successors
  BasicBlockIterator successor_begin() const {
    return this->_successors.cbegin();
  }

  /// \brief End iterator over the successors
  BasicBlockIterator successor_end() const { return this->_successors.cend(); }

  /// \brief Return the number of predecessors
  std::size_t num_predecessors() const { return this->_predecessors.size(); }

  /// \brief Begin iterator over the predecessors
  BasicBlockIterator predecessor_begin() const {
    return this->_predecessors.cbegin();
  }

  /// \brief End iterator over the predecessors
  BasicBlockIterator predecessor_end() const {
    return this->_predecessors.cend();
  }

  /// \brief Is the given basic block a successor?
  bool is_successor(BasicBlock* bb) const;

  /// \brief Is the given basic block a predecessor?
  bool is_predecessor(BasicBlock* bb) const;

  /// \brief Add the given basic block as a successor
  void add_successor(BasicBlock* bb);

  /// \brief Add the given basic block as a predecessor
  void add_predecessor(BasicBlock* bb);

  /// \brief Remove the given basic block from the list of successors
  void remove_successor(BasicBlock* bb);

  /// \brief Remove all successors
  void clear_successors();

  /// \brief Remove the given basic block from the list of predecessors
  void remove_predecessor(BasicBlock* bb);

  /// \brief Remove all predecessors
  void clear_predecessors();

  /// \brief Does the basic block have a name?
  bool has_name() const { return !this->_name.empty(); }

  /// \brief Get the name
  const std::string& name() const {
    ikos_assert_msg(!this->_name.empty(), "basic block has no name");
    return this->_name;
  }

  /// \brief Get the name
  ///
  /// Returns an empty string if the basic block has no name.
  const std::string& name_or_empty() const { return this->_name; }

  /// \brief Set the name
  void set_name(std::string name);

  /// \brief Dump the basic block for debugging purpose
  void dump(std::ostream&) const;

  /// \brief Dump the basic block and its content, for debugging purpose
  void full_dump(std::ostream&) const;

}; // end class BasicBlock

/// \brief Code
///
/// A code represents the control flow graph of a function or global variable
/// initializer
class Code : public Traceable {
private:
  // List of basic blocks
  std::vector< std::unique_ptr< BasicBlock > > _blocks;

  // List of internal variables
  std::vector< std::unique_ptr< InternalVariable > > _internal_vars;

  // Entry block
  BasicBlock* _entry_block;

  // Exit block, or null
  BasicBlock* _exit_block;

  // Parent function, or null
  Function* _function;

  // Parent global variable, or null
  GlobalVariable* _global_var;

  // Parent bundle (non-null)
  Bundle* _bundle;

public:
  /// \brief Iterator over a list of basic block
  using BasicBlockIterator = boost::transform_iterator<
      SeqExposeRawPtr< BasicBlock >,
      std::vector< std::unique_ptr< BasicBlock > >::const_iterator >;

  /// \brief Iterator over a list of internal variables
  using InternalVariableIterator = boost::transform_iterator<
      SeqExposeRawPtr< InternalVariable >,
      std::vector< std::unique_ptr< InternalVariable > >::const_iterator >;

private:
  /// \brief Private constructor
  explicit Code(Function* function);

  /// \brief Private constructor
  explicit Code(GlobalVariable* gv);

public:
  /// \brief No copy constructor
  Code(const Code&) = delete;

  /// \brief No move constructor
  Code(Code&&) = delete;

  /// \brief No copy assignment operator
  Code& operator=(const Code&) = delete;

  /// \brief No move assignment operator
  Code& operator=(Code&&) = delete;

  /// \brief Destructor
  ~Code();

  /// \brief Begin iterator over the list of basic blocks
  BasicBlockIterator begin() const {
    return boost::make_transform_iterator(this->_blocks.cbegin(),
                                          SeqExposeRawPtr< BasicBlock >());
  }

  /// \brief End iterator over the list of basic blocks
  BasicBlockIterator end() const {
    return boost::make_transform_iterator(this->_blocks.cend(),
                                          SeqExposeRawPtr< BasicBlock >());
  }

  /// \brief Begin iterator over the list of internal variables
  InternalVariableIterator internal_variable_begin() const {
    return boost::make_transform_iterator(this->_internal_vars.cbegin(),
                                          SeqExposeRawPtr<
                                              InternalVariable >());
  }

  /// \brief End iterator over the list of internal variables
  InternalVariableIterator internal_variable_end() const {
    return boost::make_transform_iterator(this->_internal_vars.cend(),
                                          SeqExposeRawPtr<
                                              InternalVariable >());
  }

  /// \brief Does it have an entry block?
  ///
  /// This should always return true, except if we just created this code and
  /// didn't set the entry block yet.
  bool has_entry_block() const { return this->_entry_block != nullptr; }

  /// \brief Get the entry basic block
  BasicBlock* entry_block() const {
    ikos_assert_msg(this->_entry_block, "code has no entry block");
    return this->_entry_block;
  }

  /// \brief Set the entry basic block
  void set_entry_block(BasicBlock* bb);

  /// \brief Does it have an exit block?
  bool has_exit_block() const { return this->_exit_block != nullptr; }

  /// \brief Get the exit basic block
  BasicBlock* exit_block() const {
    ikos_assert_msg(this->_exit_block, "code has no exit block");
    return this->_exit_block;
  }

  /// \brief Get the exit basic block, or null if there is none
  BasicBlock* exit_block_or_null() const { return this->_exit_block; }

  /// \brief Set the exit basic block
  void set_exit_block(BasicBlock* bb);

  /// \brief Is it the body of a function?
  bool is_function_body() const { return this->_function != nullptr; }

  /// \brief Get the parent function
  Function* function() const {
    ikos_assert_msg(this->_function, "code is a global variable initializer");
    return this->_function;
  }

  /// \brief Get the parent function if it is a body, or nullptr
  Function* function_or_null() const { return this->_function; }

  /// \brief Is it the code for a global variable initializer?
  bool is_global_var_initializer() const {
    return this->_global_var != nullptr;
  }

  /// \brief Get the parent global variable
  GlobalVariable* global_var() const {
    ikos_assert_msg(this->_global_var, "code is a function code");
    return this->_global_var;
  }

  /// \brief Get the parent global variable if it is an initializer, or nullptr
  GlobalVariable* global_var_or_null() const { return this->_global_var; }

  /// \brief Get the parent context
  Context& context() const { return this->bundle()->context(); }

  /// \brief Get the parent bundle
  Bundle* bundle() const { return this->_bundle; }

private:
  /// \brief Add a basic block in the code
  ///
  /// \returns a pointer on the basic block
  BasicBlock* add_basic_block(std::unique_ptr< BasicBlock >);

public:
  /// \brief Remove the given basic block
  ///
  /// Using the basic block after calling erase_basic_block() is an undefined
  /// behaviour
  void erase_basic_block(BasicBlock*);

private:
  /// \brief Add an internal variable in the code
  ///
  /// \returns a pointer on the interval variable
  InternalVariable* add_internal_variable(std::unique_ptr< InternalVariable >);

  // friends
  friend class Function;
  friend class BasicBlock;
  friend class GlobalVariable;
  friend class InternalVariable;

}; // end class Code

} // end namespace ar
} // end namespace ikos

namespace ikos {
namespace core {

/// \brief Implement GraphTraits for ar::Code*
template <>
struct GraphTraits< ar::Code* > {
  using NodeRef = ar::BasicBlock*;
  using SuccessorNodeIterator = ar::BasicBlock::BasicBlockIterator;
  using PredecessorNodeIterator = ar::BasicBlock::BasicBlockIterator;

  static ar::BasicBlock* entry(ar::Code* code) { return code->entry_block(); }

  static SuccessorNodeIterator successor_begin(ar::BasicBlock* bb) {
    return bb->successor_begin();
  }

  static SuccessorNodeIterator successor_end(ar::BasicBlock* bb) {
    return bb->successor_end();
  }

  static PredecessorNodeIterator predecessor_begin(ar::BasicBlock* bb) {
    return bb->predecessor_begin();
  }

  static PredecessorNodeIterator predecessor_end(ar::BasicBlock* bb) {
    return bb->predecessor_end();
  }
};

} // end namespace core
} // end namespace ikos
