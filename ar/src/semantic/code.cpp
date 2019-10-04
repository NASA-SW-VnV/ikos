/*******************************************************************************
 *
 * \file
 * \brief Control flow graph implementation
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

#include <ikos/ar/semantic/bundle.hpp>
#include <ikos/ar/semantic/code.hpp>
#include <ikos/ar/semantic/function.hpp>
#include <ikos/ar/semantic/statement.hpp>
#include <ikos/ar/semantic/value.hpp>

namespace ikos {
namespace ar {

// BasicBlock

BasicBlock::BasicBlock(Code* code) : _parent(code) {
  ikos_assert_msg(code, "code is null");
}

BasicBlock::~BasicBlock() = default;

BasicBlock* BasicBlock::create(Code* code) {
  auto bb = std::unique_ptr< BasicBlock >(new BasicBlock(code));
  return code->add_basic_block(std::move(bb));
}

Context& BasicBlock::context() const {
  return this->bundle()->context();
}

Bundle* BasicBlock::bundle() const {
  return this->code()->bundle();
}

void BasicBlock::push_front(std::unique_ptr< Statement > stmt) {
  stmt->set_parent(this);
  this->_statements.insert(this->_statements.begin(), std::move(stmt));
}

void BasicBlock::push_back(std::unique_ptr< Statement > stmt) {
  stmt->set_parent(this);
  this->_statements.emplace_back(std::move(stmt));
}

BasicBlock::StatementIterator BasicBlock::insert_before(
    StatementIterator it, std::unique_ptr< Statement > stmt) {
  stmt->set_parent(this);
  auto new_it = this->_statements.insert(it.base(), std::move(stmt));
  return boost::make_transform_iterator(new_it, SeqExposeRawPtr< Statement >());
}

BasicBlock::StatementIterator BasicBlock::insert_after(
    StatementIterator it, std::unique_ptr< Statement > stmt) {
  ikos_assert(it != this->end());
  stmt->set_parent(this);
  auto new_it = this->_statements.insert(std::next(it).base(), std::move(stmt));
  return boost::make_transform_iterator(new_it, SeqExposeRawPtr< Statement >());
}

std::unique_ptr< Statement > BasicBlock::replace(
    StatementIterator it, std::unique_ptr< Statement > stmt) {
  ikos_assert(it != this->end());

  // Create a non-const iterator on the statement
  auto index = std::distance(this->_statements.cbegin(), it.base());
  auto stmt_it = std::next(this->_statements.begin(), index);

  // Remove current statement
  std::unique_ptr< Statement > old = std::move(*stmt_it);
  old->set_parent(nullptr);

  // Add the new statement
  stmt->set_parent(this);
  *stmt_it = std::move(stmt);

  return old;
}

BasicBlock::StatementIterator BasicBlock::remove(StatementIterator it) {
  ikos_assert(it != this->end());
  (*it.base())->set_parent(nullptr);
  auto new_it = this->_statements.erase(it.base());
  return boost::make_transform_iterator(new_it, SeqExposeRawPtr< Statement >());
}

std::unique_ptr< Statement > BasicBlock::pop_back() {
  ikos_assert_msg(!this->_statements.empty(), "basic block is empty");
  std::unique_ptr< Statement > stmt = std::move(this->_statements.back());
  this->_statements.pop_back();
  stmt->set_parent(nullptr);
  return stmt;
}

void BasicBlock::clear_statements() {
  this->_statements.clear();
}

bool BasicBlock::is_successor(BasicBlock* bb) const {
  return std::find(this->_successors.begin(), this->_successors.end(), bb) !=
         this->_successors.end();
}

bool BasicBlock::is_predecessor(BasicBlock* bb) const {
  return std::find(this->_predecessors.begin(),
                   this->_predecessors.end(),
                   bb) != this->_predecessors.end();
}

void BasicBlock::add_successor(BasicBlock* bb) {
  if (!this->is_successor(bb)) {
    this->_successors.push_back(bb);
    bb->_predecessors.push_back(this);
  }
}

void BasicBlock::add_predecessor(BasicBlock* bb) {
  if (!this->is_predecessor(bb)) {
    this->_predecessors.push_back(bb);
    bb->_successors.push_back(this);
  }
}

void BasicBlock::remove_successor(BasicBlock* bb) {
  if (this->is_successor(bb)) {
    this->_successors.erase(std::remove(this->_successors.begin(),
                                        this->_successors.end(),
                                        bb),
                            this->_successors.end());
    bb->_predecessors.erase(std::remove(bb->_predecessors.begin(),
                                        bb->_predecessors.end(),
                                        this),
                            bb->_predecessors.end());
  }
}

void BasicBlock::clear_successors() {
  // Remove this from predecessors of our successors
  for (BasicBlock* succ : this->_successors) {
    succ->_predecessors.erase(std::remove(succ->_predecessors.begin(),
                                          succ->_predecessors.end(),
                                          this),
                              succ->_predecessors.end());
  }

  this->_successors.clear();
}

void BasicBlock::remove_predecessor(BasicBlock* bb) {
  if (this->is_predecessor(bb)) {
    this->_predecessors.erase(std::remove(this->_predecessors.begin(),
                                          this->_predecessors.end(),
                                          bb),
                              this->_predecessors.end());
    bb->_successors.erase(std::remove(bb->_successors.begin(),
                                      bb->_successors.end(),
                                      this),
                          bb->_successors.end());
  }
}

void BasicBlock::clear_predecessors() {
  // Remove this from successors of our predecessors
  for (BasicBlock* pred : this->_predecessors) {
    pred->_successors.erase(std::remove(pred->_successors.begin(),
                                        pred->_successors.end(),
                                        this),
                            pred->_successors.end());
  }

  this->_predecessors.clear();
}

void BasicBlock::set_name(std::string name) {
  this->_name = std::move(name);
}

void BasicBlock::dump(std::ostream& o) const {
  o << "#";
  if (this->has_name()) {
    o << this->name();
  } else {
    o << this;
  }
}

void BasicBlock::full_dump(std::ostream& o) const {
  // name
  this->dump(o);

  // predecessors
  o << " predecessors={";
  for (auto it = this->_predecessors.cbegin(), et = this->_predecessors.cend();
       it != et;) {
    BasicBlock* pred = *it;
    pred->dump(o);
    o << "#";
    ++it;
    if (it != et) {
      o << ", ";
    }
  }
  o << "}";

  // successors
  o << " successors={";
  for (auto it = this->_successors.cbegin(), et = this->_successors.cend();
       it != et;) {
    BasicBlock* succ = *it;
    succ->dump(o);
    ++it;
    if (it != et) {
      o << ", ";
    }
  }
  o << "}";

  // statements
  o << " {\n";
  for (const auto& stmt : this->_statements) {
    o << "  ";
    stmt->dump(o);
    o << "\n";
  }
  o << "}\n";
}

// Code

Code::Code(Function* function)
    : _entry_block(nullptr),
      _exit_block(nullptr),
      _function(function),
      _global_var(nullptr),
      _bundle(function->bundle()) {
  ikos_assert_msg(function, "function is null");
}

Code::Code(GlobalVariable* gv)
    : _entry_block(nullptr),
      _exit_block(nullptr),
      _function(nullptr),
      _global_var(gv),
      _bundle(gv->bundle()) {
  ikos_assert_msg(gv, "gv is null");
}

Code::~Code() = default;

void Code::set_entry_block(BasicBlock* bb) {
  this->_entry_block = bb;
}

void Code::set_exit_block(BasicBlock* bb) {
  this->_exit_block = bb;
}

BasicBlock* Code::add_basic_block(std::unique_ptr< BasicBlock > bb) {
  this->_blocks.emplace_back(std::move(bb));
  return this->_blocks.back().get();
}

void Code::erase_basic_block(BasicBlock* bb) {
  ikos_assert_msg(this->_entry_block != bb, "cannot erase the entry block");
  ikos_assert_msg(this->_exit_block != bb, "cannot erase the exit block");

  bb->clear_statements();
  bb->clear_predecessors();
  bb->clear_successors();

  this->_blocks.erase(std::remove_if(this->_blocks.begin(),
                                     this->_blocks.end(),
                                     [=](const auto& bb_ptr) {
                                       return bb_ptr.get() == bb;
                                     }),
                      this->_blocks.end());
}

InternalVariable* Code::add_internal_variable(
    std::unique_ptr< InternalVariable > iv) {
  this->_internal_vars.emplace_back(std::move(iv));
  return this->_internal_vars.back().get();
}

} // end namespace ar
} // end namespace ikos
