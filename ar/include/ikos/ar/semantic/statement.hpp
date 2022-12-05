/*******************************************************************************
 *
 * \file
 * \brief Statement definitions
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
#include <utility>
#include <vector>

#include <ikos/core/adt/small_vector.hpp>

#include <ikos/ar/semantic/bundle.hpp>
#include <ikos/ar/semantic/code.hpp>
#include <ikos/ar/semantic/context.hpp>
#include <ikos/ar/semantic/function.hpp>
#include <ikos/ar/semantic/intrinsic.hpp>
#include <ikos/ar/semantic/type.hpp>
#include <ikos/ar/semantic/value.hpp>
#include <ikos/ar/support/assert.hpp>
#include <ikos/ar/support/number.hpp>
#include <ikos/ar/support/traceable.hpp>

namespace ikos {
namespace ar {

/// \brief Base class for statements
class Statement : public Traceable {
public:
  enum StatementKind {
    AssignmentKind,
    UnaryOperationKind,
    BinaryOperationKind,
    ComparisonKind,
    ReturnValueKind,
    UnreachableKind,
    AllocateKind,
    PointerShiftKind,
    LoadKind,
    StoreKind,
    ExtractElementKind,
    InsertElementKind,
    ShuffleVectorKind,
    _BeginCallBaseKind,
    CallKind,
    InvokeKind,
    _EndCallBaseKind,
    LandingPadKind,
    ResumeKind,
  };

public:
  /// \brief List of operands
  using Operands = core::SmallVector< Value*, 2 >;

  /// \brief Iterator on operands
  using OpIterator = Operands::const_iterator;

protected:
  // Kind of statement
  StatementKind _kind;

  // Parent basic block
  BasicBlock* _parent;

  // Result variable (or nullptr)
  Variable* _result;

  // Operands
  Operands _operands;

protected:
  /// \brief Protected constructor
  Statement(StatementKind kind, Variable* result, Operands operands);

  /// \brief Protected constructor
  Statement(StatementKind kind,
            Variable* result,
            std::initializer_list< Value* > operands);

public:
  /// \brief No copy constructor
  Statement(const Statement&) = delete;

  /// \brief No move constructor
  Statement(Statement&&) = delete;

  /// \brief No copy assignment operator
  Statement& operator=(const Statement&) = delete;

  /// \brief No move assignment operator
  Statement& operator=(Statement&&) = delete;

  /// \brief Destructor
  virtual ~Statement();

  /// \brief Get the kind of statement
  StatementKind kind() const { return this->_kind; }

  /// \brief Get the parent context
  Context& context() const { return this->bundle()->context(); }

  /// \brief Get the parent bundle
  Bundle* bundle() const { return this->code()->bundle(); }

  /// \brief Get the parent code
  Code* code() const { return this->parent()->code(); }

  /// \brief Does it have a parent basic block?
  bool has_parent() const { return this->_parent != nullptr; }

  /// \brief Get the parent basic block
  BasicBlock* parent() const {
    ikos_assert_msg(this->has_parent(), "statement has no parent");
    return this->_parent;
  }

  /// \brief Return an iterator on the statement in the parent basic block
  BasicBlock::StatementIterator iterator() const;

  /// \brief Return the previous statement in the parent basic block, or nullptr
  Statement* prev_statement() const;

  /// \brief Return the next statement in the parent basic block, or nullptr
  Statement* next_statement() const;

  /// \brief Does it have a result variable?
  bool has_result() const { return this->_result != nullptr; }

  /// \brief Get the result variable
  Variable* result() const {
    ikos_assert_msg(this->has_result(), "statement has no result");
    return this->_result;
  }

  /// \brief Get the result variable, or null if there is no result
  Variable* result_or_null() const { return this->_result; }

  /// \brief Get the number of operands
  std::size_t num_operands() const { return this->_operands.size(); }

  /// \brief Begin iterator over the operands
  OpIterator op_begin() const { return this->_operands.cbegin(); }

  /// \brief End iterator over the operands
  OpIterator op_end() const { return this->_operands.cend(); }

  /// \brief Get the n-th operand
  Value* operand(std::size_t i) const {
    ikos_assert_msg(i < this->_operands.size(), "invalid index");
    return this->_operands[i];
  }

  /// \brief Return true if one operand is ar::UndefinedConstant
  bool has_undefined_constant_operand() const;

  /// \brief Dump the statement for debugging purpose
  virtual void dump(std::ostream&) const = 0;

  /// \brief Clone the statement
  ///
  /// Returns a fresh statement thas has no parent
  virtual std::unique_ptr< Statement > clone() const = 0;

private:
  /// \brief Set the parent basic block
  void set_parent(BasicBlock* parent);

  // friends
  friend class BasicBlock;

}; // end class Statement

/// \brief Assignment statement
///
/// It represents an assignment statement,
/// where the left-hand-side is an InternalVariable.
class Assignment final : public Statement {
private:
  /// \brief Private constructor
  Assignment(InternalVariable* result, Value* operand);

public:
  /// \brief Static constructor
  static std::unique_ptr< Assignment > create(InternalVariable* result,
                                              Value* operand);

  /// \brief Get the result variable
  InternalVariable* result() const {
    return cast< InternalVariable >(this->_result);
  }

  /// \brief Get the operand
  Value* operand() const { return this->_operands[0]; }

  /// \brief Dump the statement for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Clone the statement
  std::unique_ptr< Statement > clone() const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Statement* s) {
    return s->kind() == AssignmentKind;
  }

}; // end class Assignment

/// \brief Unary operation
class UnaryOperation final : public Statement {
public:
  /// \brief List of operators
  enum Operator {
    UTrunc,
    STrunc,
    ZExt,
    SExt,
    FPTrunc,
    FPExt,
    FPToUI,
    FPToSI,
    UIToFP,
    SIToFP,
    PtrToUI,
    PtrToSI,
    UIToPtr,
    SIToPtr,
    Bitcast
  };

private:
  // Operator
  Operator _op;

private:
  /// \brief Private constructor
  UnaryOperation(Operator op, InternalVariable* result, Value* operand);

public:
  /// \brief Static constructor
  static std::unique_ptr< UnaryOperation > create(Operator op,
                                                  InternalVariable* result,
                                                  Value* operand);

  /// \brief Get the operator
  Operator op() const { return this->_op; }

  /// \brief Get the result variable
  InternalVariable* result() const {
    return cast< InternalVariable >(this->_result);
  }

  /// \brief Get the operand
  Value* operand() const { return this->_operands[0]; }

  /// \brief Get a textual representation of the given operator
  static std::string operator_text(Operator op);

  /// \brief Dump the statement for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Clone the statement
  std::unique_ptr< Statement > clone() const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Statement* s) {
    return s->kind() == UnaryOperationKind;
  }

}; // end class UnaryOperation

/// \brief Binary operation
class BinaryOperation final : public Statement {
public:
  /// \brief List of operators
  enum Operator {
    _BeginIntegerOp,
    _BeginUnsignedIntegerOp,
    UAdd,
    USub,
    UMul,
    UDiv,
    URem,
    UShl,
    ULShr,
    UAShr,
    UAnd,
    UOr,
    UXor,
    _EndUnsignedIntegerOp,
    _BeginSignedIntegerOp,
    SAdd,
    SSub,
    SMul,
    SDiv,
    SRem,
    SShl,
    SLShr,
    SAShr,
    SAnd,
    SOr,
    SXor,
    _EndSignedIntegerOp,
    _EndIntegerOp,
    _BeginFloatOp,
    FAdd,
    FSub,
    FMul,
    FDiv,
    FRem,
    _EndFloatOp,
  };

private:
  /// \brief Operator
  Operator _op;

  /// \brief No wrap flag
  ///
  /// Indicate that the operation must not wrap, i.e, if an integer overflow
  /// occurs, it is considered an error that traps the program.
  ///
  /// This flag is available for operators:
  /// UAdd, SAdd, USub, SSub, UMul, SMul, UShl and SShl
  bool _no_wrap : 1;

  /// \brief Exact flag
  ///
  /// Indicate that if the integer operation is not exact, it is considered an
  /// error that traps the program.
  ///
  /// This flag is available for operators:
  /// UDiv, SDiv, ULShr, SLShr, UAShr and SAShr
  bool _exact : 1;

private:
  /// \brief Private constructor
  BinaryOperation(Operator op,
                  InternalVariable* result,
                  Value* left,
                  Value* right,
                  bool no_wrap,
                  bool exact);

public:
  /// \brief Static constructor
  static std::unique_ptr< BinaryOperation > create(Operator op,
                                                   InternalVariable* result,
                                                   Value* left,
                                                   Value* right,
                                                   bool no_wrap = false,
                                                   bool exact = false);

  /// \brief Get the operator
  Operator op() const { return this->_op; }

  /// \brief Get the result variable
  InternalVariable* result() const {
    return cast< InternalVariable >(this->_result);
  }

  /// \brief Get the left operand (index 0)
  Value* left() const { return this->_operands[0]; }

  /// \brief Get the right operand (index 1)
  Value* right() const { return this->_operands[1]; }

  /// \brief Is it an integer operation?
  bool is_integer_op() const {
    return this->_op >= _BeginIntegerOp && this->_op <= _EndIntegerOp;
  }

  /// \brief Is it an unsigned integer operation?
  bool is_unsigned_op() const {
    return this->_op >= _BeginUnsignedIntegerOp &&
           this->_op <= _EndUnsignedIntegerOp;
  }

  /// \brief Is it a signed integer operation?
  bool is_signed_op() const {
    return this->_op >= _BeginSignedIntegerOp &&
           this->_op <= _EndSignedIntegerOp;
  }

  /// \brief Is it a floating point operation?
  bool is_float_op() const {
    return this->_op >= _BeginFloatOp && this->_op <= _EndFloatOp;
  }

  /// \brief Check if the given operator wraps integers
  static bool is_wrapping_operator(Operator op) {
    return op == UAdd || op == SAdd || op == USub || op == SSub || op == UMul ||
           op == SMul || op == UShl || op == SShl;
  }

  /// \brief Check if the given operator can be exact
  static bool is_exact_operator(Operator op) {
    return op == UDiv || op == SDiv || op == ULShr || op == SLShr ||
           op == UAShr || op == SAShr;
  }

  /// \brief Check if the operation must not wrap integers
  ///
  /// In this case, If an integer overflow occurs, it is considered an error
  /// that traps the program.
  bool has_no_wrap() const { return this->_no_wrap; }

  /// \brief Check if the operation must be exact
  ///
  /// In this case, If the integer operation is not exact, it is considered an
  /// error that traps the program.
  bool is_exact() const { return this->_exact; }

  /// \brief Get a textual representation of the given operator
  static std::string operator_text(Operator op);

  /// \brief Dump the statement for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Clone the statement
  std::unique_ptr< Statement > clone() const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Statement* s) {
    return s->kind() == BinaryOperationKind;
  }

}; // end class BinaryOperation

/// \brief Comparison statement
class Comparison final : public Statement {
public:
  /// \brief List of predicates
  enum Predicate {
    _BeginIntegerPred,
    _BeginUnsignedIntegerPred,
    UIEQ,
    UINE,
    UIGT,
    UIGE,
    UILT,
    UILE,
    _EndUnsignedIntegerPred,
    _BeginSignedIntegerPred,
    SIEQ,
    SINE,
    SIGT,
    SIGE,
    SILT,
    SILE,
    _EndSignedIntegerPred,
    _EndIntegerPred,
    _BeginFloatPred,
    FOEQ,
    FOGT,
    FOGE,
    FOLT,
    FOLE,
    FONE,
    FORD,
    FUNO,
    FUEQ,
    FUGT,
    FUGE,
    FULT,
    FULE,
    FUNE,
    _EndFloatPred,
    _BeginPointerPred,
    PEQ,
    PNE,
    PGT,
    PGE,
    PLT,
    PLE,
    _EndPointerPred,
  };

private:
  // Predicate
  Predicate _predicate;

private:
  /// \brief Private constructor
  Comparison(Predicate predicate, Value* left, Value* right);

public:
  /// \brief Static constructor
  static std::unique_ptr< Comparison > create(Predicate predicate,
                                              Value* left,
                                              Value* right);

  /// \brief Get the predicate
  Predicate predicate() const { return this->_predicate; }

  /// \brief Get the left operand (index 0)
  Value* left() const { return this->_operands[0]; }

  /// \brief Get the right operand (index 1)
  Value* right() const { return this->_operands[1]; }

  /// \brief Is it an integer comparison?
  bool is_integer_predicate() const {
    return this->_predicate >= _BeginIntegerPred &&
           this->_predicate <= _EndIntegerPred;
  }

  /// \brief Is it an unsigned integer comparison?
  bool is_unsigned_predicate() const {
    return this->_predicate >= _BeginUnsignedIntegerPred &&
           this->_predicate <= _EndUnsignedIntegerPred;
  }

  /// \brief Is it a signed integer comparison?
  bool is_signed_predicate() const {
    return this->_predicate >= _BeginSignedIntegerPred &&
           this->_predicate <= _EndSignedIntegerPred;
  }

  /// \brief Is it a floating point comparison?
  bool is_float_predicate() const {
    return this->_predicate >= _BeginFloatPred &&
           this->_predicate <= _EndFloatPred;
  }

  /// \brief Is it a pointer comparison?
  bool is_pointer_predicate() const {
    return this->_predicate >= _BeginPointerPred &&
           this->_predicate <= _EndPointerPred;
  }

  /// \brief Get a textual representation of the given predicate
  static std::string predicate_text(Predicate pred);

  /// \brief Get the opposite predicate
  ///
  /// For example:
  ///   inverse_predicate(uieq) = uine
  ///   inverse_predicate(silt) = sige
  static Predicate inverse_predicate(Predicate pred);

  /// \brief Create the opposite comparison statement
  ///
  /// For example:
  ///   inverse(x uieq y) -> x uine y
  ///   inverse(x silt y) -> x sige y
  std::unique_ptr< Comparison > inverse() const;

  /// \brief Dump the statement for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Clone the statement
  std::unique_ptr< Statement > clone() const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Statement* s) {
    return s->kind() == ComparisonKind;
  }

}; // end class Comparison

/// \brief Return statement
class ReturnValue final : public Statement {
private:
  /// \brief Private constructor
  explicit ReturnValue(Value* operand);

public:
  /// \brief Static constructor
  ///
  /// \param operand The returned value, or null
  static std::unique_ptr< ReturnValue > create(Value* operand = nullptr);

  /// \brief Does it have an operand?
  bool has_operand() const { return !this->_operands.empty(); }

  /// \brief Get the operand
  Value* operand() const {
    ikos_assert_msg(this->has_operand(), "return has no operand");
    return this->_operands[0];
  }

  /// \brief Get the operand, or null if there is none
  Value* operand_or_null() const {
    if (this->_operands.empty()) {
      return nullptr;
    } else {
      return this->_operands[0];
    }
  }

  /// \brief Dump the statement for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Clone the statement
  std::unique_ptr< Statement > clone() const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Statement* s) {
    return s->kind() == ReturnValueKind;
  }

}; // end class ReturnValue

/// \brief Unreachable statement
class Unreachable final : public Statement {
private:
  /// \brief Private constructor
  Unreachable();

public:
  /// \brief Static constructor
  static std::unique_ptr< Unreachable > create();

  /// \brief Dump the statement for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Clone the statement
  std::unique_ptr< Statement > clone() const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Statement* s) {
    return s->kind() == UnreachableKind;
  }

}; // end class Unreachable

/// \brief Stack allocation statement
class Allocate final : public Statement {
private:
  // Allocated type
  Type* _allocated_type;

private:
  /// \brief Private constructor
  Allocate(LocalVariable* result, Type* allocated_type, Value* array_size);

public:
  /// \brief Static constructor
  static std::unique_ptr< Allocate > create(LocalVariable* result,
                                            Type* allocated_type,
                                            Value* array_size);

  /// \brief Get the result local variable
  LocalVariable* result() const { return cast< LocalVariable >(this->_result); }

  /// \brief Get the allocated type
  Type* allocated_type() const { return this->_allocated_type; }

  /// \brief Get the array size operand
  Value* array_size() const { return this->_operands[0]; }

  /// \brief Dump the statement for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Clone the statement
  std::unique_ptr< Statement > clone() const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Statement* s) { return s->kind() == AllocateKind; }

}; // end class Allocate

/// \brief Pointer shift statement
///
/// It represents a pointer arithmetic operation.
/// It basically represents the operation:
/// p = base + a * x + b * y + ...
///
/// Note that offsets are in bytes.
/// Factors should have type `IntegerType::size_type()`.
/// Operands can have any integer type.
class PointerShift final : public Statement {
public:
  /// \brief Term
  ///
  /// A term is the product of a constant and a value
  using Term = std::pair< MachineInt, Value* >;

private:
  /// \brief List of factors
  using Factors = core::SmallVector< MachineInt, 2 >;

  /// \brief Iterator over the factors
  using FactorIterator = Factors::const_iterator;

public:
  /// \brief Iterator on the terms
  class TermIterator
      : public boost::iterator_facade< TermIterator,
                                       const Term,
                                       boost::random_access_traversal_tag,
                                       const Term > {
  private:
    friend class boost::iterator_core_access;

  private:
    FactorIterator _factor_it;
    OpIterator _value_it;

  public:
    TermIterator(const FactorIterator& factor_it, const OpIterator& value_it)
        : _factor_it(factor_it), _value_it(value_it) {}

    void increment() {
      ++this->_factor_it;
      ++this->_value_it;
    }

    void decrement() {
      --this->_factor_it;
      --this->_value_it;
    }

    void advance(std::ptrdiff_t n) {
      std::advance(this->_factor_it, n);
      std::advance(this->_value_it, n);
    }

    bool equal(const TermIterator& o) const {
      return this->_factor_it == o._factor_it && this->_value_it == o._value_it;
    }

    std::ptrdiff_t distance_to(const TermIterator& o) const {
      return std::distance(this->_factor_it, o._factor_it);
    }

    const Term dereference() const {
      return {*this->_factor_it, *this->_value_it};
    }

    /// \brief Cannot be implemented
    const Term* operator->() const = delete;

  }; // end class TermIterator

private:
  /// \brief List of factors
  Factors _factors;

private:
  /// \brief Private constructor
  PointerShift(InternalVariable* result,
               Value* pointer,
               const std::vector< Term >& terms);

  /// \brief Private constructor
  ///
  /// Only used by clone()
  PointerShift(InternalVariable* result, Factors factors, Operands operands);

public:
  /// \brief Static constructor
  static std::unique_ptr< PointerShift > create(
      InternalVariable* result,
      Value* pointer,
      const std::vector< Term >& terms);

  /// \brief Get the result variable
  InternalVariable* result() const {
    return cast< InternalVariable >(this->_result);
  }

  /// \brief Get the base pointer
  Value* pointer() const { return this->_operands[0]; }

  /// \brief Get the number of terms
  std::size_t num_terms() const { return this->_factors.size(); }

  /// \brief Get an operand
  Term term(std::size_t i) const {
    ikos_assert_msg(i < this->num_terms(), "invalid index");
    return {this->_factors[i], this->_operands[i + 1]};
  }

  /// \brief Begin iterator over the list of operands
  TermIterator term_begin() const {
    return {this->_factors.cbegin(), this->op_begin() + 1};
  }

  /// \brief End iterator over the list of operands
  TermIterator term_end() const {
    return {this->_factors.cend(), this->op_end()};
  }

  /// \brief Dump the statement for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Clone the statement
  std::unique_ptr< Statement > clone() const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Statement* s) {
    return s->kind() == PointerShiftKind;
  }

}; // end class PointerShift

/// \brief Load statement
class Load final : public Statement {
private:
  // Alignment of the memory access, in bytes (0 if unspecified)
  uint64_t _alignment;

  // Load from a volatile memory location
  bool _is_volatile;

private:
  /// \brief Private constructor
  Load(InternalVariable* result,
       Value* operand,
       uint64_t alignment,
       bool is_volatile);

public:
  /// \brief Static constructor
  static std::unique_ptr< Load > create(InternalVariable* result,
                                        Value* operand,
                                        uint64_t alignment,
                                        bool is_volatile);

  /// \brief Get the result variable
  InternalVariable* result() const {
    return cast< InternalVariable >(this->_result);
  }

  /// \brief Get the pointer operand
  Value* operand() const { return this->_operands[0]; }

  /// \brief Get the alignment of the memory access, in bytes (0 if unspecified)
  uint64_t alignment() const { return this->_alignment; }

  /// \brief Return true if this statement has an alignment constraint
  bool has_alignment() const { return this->alignment() > 0; }

  /// \brief Return true if this is a load from a volatile memory location
  bool is_volatile() const { return this->_is_volatile; }

  /// \brief Dump the statement for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Clone the statement
  std::unique_ptr< Statement > clone() const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Statement* s) { return s->kind() == LoadKind; }

}; // end class Load

/// \brief Store statement
class Store final : public Statement {
private:
  // Alignment of the memory access, in bytes (0 if unspecified)
  uint64_t _alignment;

  // Load from a volatile memory location
  bool _is_volatile;

private:
  /// \brief Private constructor
  Store(Value* pointer, Value* value, uint64_t alignment, bool is_volatile);

public:
  /// \brief Static constructor
  static std::unique_ptr< Store > create(Value* pointer,
                                         Value* value,
                                         uint64_t alignment,
                                         bool is_volatile);

  /// \brief Get the pointer operand
  Value* pointer() const { return this->_operands[0]; }

  /// \brief Get the value operand
  Value* value() const { return this->_operands[1]; }

  /// \brief Get the alignment of the memory access, in bytes (0 if unspecified)
  uint64_t alignment() const { return this->_alignment; }

  /// \brief Return true if this statement has an alignment constraint
  bool has_alignment() const { return this->alignment() > 0; }

  /// \brief Return true if this is a store to a volatile memory location
  bool is_volatile() const { return this->_is_volatile; }

  /// \brief Dump the statement for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Clone the statement
  std::unique_ptr< Statement > clone() const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Statement* s) { return s->kind() == StoreKind; }

}; // end class Store

/// \brief Extract element from an aggregate
class ExtractElement final : public Statement {
private:
  /// \brief Private constructor
  ExtractElement(InternalVariable* result, Value* aggregate, Value* offset);

public:
  /// \brief Static constructor
  static std::unique_ptr< ExtractElement > create(InternalVariable* result,
                                                  Value* aggregate,
                                                  Value* offset);

  /// \brief Get the result variable
  InternalVariable* result() const {
    return cast< InternalVariable >(this->_result);
  }

  /// \brief Get the aggregate operand
  Value* aggregate() const { return this->_operands[0]; }

  /// \brief Get the offset operand
  Value* offset() const { return this->_operands[1]; }

  /// \brief Dump the statement for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Clone the statement
  std::unique_ptr< Statement > clone() const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Statement* s) {
    return s->kind() == ExtractElementKind;
  }

}; // end class ExtractElement

/// \brief Insert element into an aggregate
class InsertElement final : public Statement {
private:
  /// \brief Private constructor
  InsertElement(InternalVariable* result,
                Value* aggregate,
                Value* offset,
                Value* element);

public:
  /// \brief Static constructor
  static std::unique_ptr< InsertElement > create(InternalVariable* result,
                                                 Value* aggregate,
                                                 Value* offset,
                                                 Value* element);

  /// \brief Get the result variable
  InternalVariable* result() const {
    return cast< InternalVariable >(this->_result);
  }

  /// \brief Get the aggregate operand
  Value* aggregate() const { return this->_operands[0]; }

  /// \brief Get the offset operand
  Value* offset() const { return this->_operands[1]; }

  /// \brief Get the element operand
  Value* element() const { return this->_operands[2]; }

  /// \brief Dump the statement for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Clone the statement
  std::unique_ptr< Statement > clone() const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Statement* s) {
    return s->kind() == InsertElementKind;
  }

}; // end class InsertElement

/// \brief Shuffle vector statement
class ShuffleVector final : public Statement {
private:
  /// \brief Private constructor
  ShuffleVector(InternalVariable* result, Value* left, Value* right);

public:
  /// \brief Static constructor
  static std::unique_ptr< ShuffleVector > create(InternalVariable* result,
                                                 Value* left,
                                                 Value* right);

  /// \brief Get the result variable
  InternalVariable* result() const {
    return cast< InternalVariable >(this->_result);
  }

  /// \brief Get the left operand
  Value* left() const { return this->_operands[0]; }

  /// \brief Get the right operand
  Value* right() const { return this->_operands[1]; }

  /// \brief Dump the statement for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Clone the statement
  std::unique_ptr< Statement > clone() const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Statement* s) {
    return s->kind() == ShuffleVectorKind;
  }

}; // end class ShuffleVector

/// \brief Base class for Call and Invoke instructions
class CallBase : public Statement {
public:
  /// \brief Iterator over the list of arguments
  using ArgIterator = OpIterator;

protected:
  /// \brief Protected constructor
  CallBase(StatementKind kind,
           InternalVariable* result,
           Value* called,
           const std::vector< Value* >& arguments);

  /// \brief Protected constructor
  ///
  /// Helper for Call::clone() and Invoke::clone() methods
  CallBase(StatementKind kind,
           InternalVariable* result,
           const Operands& operands);

public:
  /// \brief Get the result variable
  InternalVariable* result() const {
    ikos_assert_msg(this->has_result(), "call has no result");
    return cast< InternalVariable >(this->_result);
  }

  /// \brief Get the result variable, or null if there is none
  InternalVariable* result_or_null() const {
    return cast_or_null< InternalVariable >(this->_result);
  }

  /// \brief Get the called value
  Value* called() const { return this->_operands[0]; }

  /// \brief Begin iterator over the arguments
  ArgIterator arg_begin() const { return this->op_begin() + 1; }

  /// \brief End iterator over the arguments
  ArgIterator arg_end() const { return this->op_end(); }

  /// \brief Get the number of arguments
  std::size_t num_arguments() const { return this->num_operands() - 1; }

  /// \brief Get the n-th argument
  Value* argument(std::size_t i) const {
    ikos_assert_msg(i < this->num_arguments(), "invalid index");
    return this->_operands[i + 1];
  }

  /// \brief Is it a call to assembly code?
  bool is_asm() const { return isa< InlineAssemblyConstant >(this->called()); }

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Statement* s) {
    return s->kind() >= _BeginCallBaseKind && s->kind() <= _EndCallBaseKind;
  }

}; // end class CallBase

/// \brief Call instruction
class Call : public CallBase {
protected:
  /// \brief Private constructor
  Call(InternalVariable* result,
       Value* called,
       const std::vector< Value* >& arguments);

  /// \brief Private constructor
  ///
  /// Only used by clone()
  Call(InternalVariable* result, const Operands& operands);

public:
  /// \brief Static constructor
  ///
  /// \param result The result variable, or null
  /// \param called The called value (e.g, a FunctionPointerConstant)
  /// \param arguments The list of arguments
  static std::unique_ptr< Call > create(InternalVariable* result,
                                        Value* called,
                                        const std::vector< Value* >& arguments);

  /// \brief Static constructor
  ///
  /// \param result The result variable, or null
  /// \param function The called function
  /// \param arguments The list of arguments
  static std::unique_ptr< Call > create(InternalVariable* result,
                                        Function* function,
                                        const std::vector< Value* >& arguments);

  /// \brief Dump the statement for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Clone the statement
  std::unique_ptr< Statement > clone() const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Statement* s) { return s->kind() == CallKind; }

}; // end class Call

/// \brief Intrinsic call statement
class IntrinsicCall : public Call {
protected:
  /// \brief Protected constructor
  IntrinsicCall(InternalVariable* result,
                Function* function,
                const std::vector< Value* >& arguments);

public:
  /// \brief Get the called function pointer
  FunctionPointerConstant* called() const {
    return cast< FunctionPointerConstant >(this->_operands[0]);
  }

  /// \brief Get the called function
  Function* called_function() const { return this->called()->function(); }

  /// \brief Get the intrinsic ID
  Intrinsic::ID intrinsic_id() const {
    return this->called_function()->intrinsic_id();
  }

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Statement* s) {
    return isa< Call >(s) && classof(cast< Call >(s));
  }

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Call* s) {
    return isa< FunctionPointerConstant >(s->called()) &&
           cast< FunctionPointerConstant >(s->called())
               ->function()
               ->is_intrinsic();
  }

}; // end class IntrinsicCall

/// \brief Invoke statement
class Invoke : public CallBase {
private:
  // Normal basic block destination
  BasicBlock* _normal_dest;

  // Exception basic block destination
  BasicBlock* _exception_dest;

private:
  /// \brief Private constructor
  Invoke(InternalVariable* result,
         Value* called,
         const std::vector< Value* >& arguments,
         BasicBlock* normal_dest,
         BasicBlock* exception_dest);

  /// \brief Private constructor
  ///
  /// Only used by clone()
  Invoke(InternalVariable* result,
         const Operands& operands,
         BasicBlock* normal_dest,
         BasicBlock* exception_dest);

public:
  /// \brief Static constructor
  ///
  /// \param result The result variable, or null
  /// \param called The called value (e.g, a FunctionPointerConstant)
  /// \param arguments The list of arguments
  /// \param normal_dest The normal basic block destination
  /// \param exception_dest The exception basic block destination
  static std::unique_ptr< Invoke > create(
      InternalVariable* result,
      Value* called,
      const std::vector< Value* >& arguments,
      BasicBlock* normal_dest,
      BasicBlock* exception_dest);

  /// \brief Static constructor
  ///
  /// \param result The result variable, or null
  /// \param function The called function
  /// \param arguments The list of arguments
  /// \param normal_dest The normal basic block destination
  /// \param exception_dest The exception basic block destination
  static std::unique_ptr< Invoke > create(
      InternalVariable* result,
      Function* function,
      const std::vector< Value* >& arguments,
      BasicBlock* normal_dest,
      BasicBlock* exception_dest);

  /// \brief Get the normal basic block destination
  BasicBlock* normal_dest() const { return this->_normal_dest; }

  /// \brief Set the normal basic block destination
  void set_normal_dest(BasicBlock* bb) { this->_normal_dest = bb; }

  /// \brief Get the exception basic block destination
  BasicBlock* exception_dest() const { return this->_exception_dest; }

  /// \brief Set the exception basic block destination
  void set_exception_dest(BasicBlock* bb) { this->_exception_dest = bb; }

  /// \brief Dump the statement for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Clone the statement
  std::unique_ptr< Statement > clone() const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Statement* s) { return s->kind() == InvokeKind; }

}; // end class Invoke

/// \brief Intrinsic invoke statement
class IntrinsicInvoke : public Invoke {
public:
  /// \brief Get the called function pointer
  FunctionPointerConstant* called() const {
    return cast< FunctionPointerConstant >(this->_operands[0]);
  }

  /// \brief Get the called function
  Function* called_function() const { return this->called()->function(); }

  /// \brief Get the intrinsic ID
  Intrinsic::ID intrinsic_id() const {
    return this->called_function()->intrinsic_id();
  }

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Statement* s) {
    return isa< Invoke >(s) && classof(cast< Invoke >(s));
  }

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Invoke* s) {
    return isa< FunctionPointerConstant >(s->called()) &&
           cast< FunctionPointerConstant >(s->called())
               ->function()
               ->is_intrinsic();
  }

}; // end class IntrinsicInvoke

/// \brief Memory copy statement (memcpy)
class MemoryCopy final : public IntrinsicCall {
private:
  /// \brief Private constructor
  MemoryCopy(Bundle* bundle,
             Value* destination,
             Value* source,
             Value* length,
             uint64_t destination_alignment,
             uint64_t source_alignment,
             bool is_volatile);

public:
  /// \brief Static constructor
  static std::unique_ptr< MemoryCopy > create(Bundle* bundle,
                                              Value* destination,
                                              Value* source,
                                              Value* length,
                                              uint64_t destination_alignment,
                                              uint64_t source_alignment,
                                              bool is_volatile);

  /// \brief Get the destination operand
  Value* destination() const { return this->argument(0); }

  /// \brief Get the source operand
  Value* source() const { return this->argument(1); }

  /// \brief Get the length operand
  Value* length() const { return this->argument(2); }

  /// \brief Get the alignment of the destination, in bytes (0 if unspecified)
  uint64_t destination_alignment() const {
    return cast< IntegerConstant >(this->argument(3))->value().to< uint64_t >();
  }

  /// \brief Get the alignment of the source, in bytes (0 if unspecified)
  uint64_t source_alignment() const {
    return cast< IntegerConstant >(this->argument(4))->value().to< uint64_t >();
  }

  /// \brief Return true if the source or destination is volatile
  bool is_volatile() const {
    return cast< IntegerConstant >(this->argument(5))->value() != 0;
  }

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Statement* s) {
    return isa< IntrinsicCall >(s) &&
           cast< IntrinsicCall >(s)->intrinsic_id() == Intrinsic::MemoryCopy;
  }

}; // end class MemoryCopy

/// \brief Memory move statement (memmove)
class MemoryMove final : public IntrinsicCall {
private:
  /// \brief Private constructor
  MemoryMove(Bundle* bundle,
             Value* destination,
             Value* source,
             Value* length,
             uint64_t destination_alignment,
             uint64_t source_alignment,
             bool is_volatile);

public:
  /// \brief Static constructor
  static std::unique_ptr< MemoryMove > create(Bundle* bundle,
                                              Value* destination,
                                              Value* source,
                                              Value* length,
                                              uint64_t destination_alignment,
                                              uint64_t source_alignment,
                                              bool is_volatile);

  /// \brief Get the destination operand
  Value* destination() const { return this->argument(0); }

  /// \brief Get the source operand
  Value* source() const { return this->argument(1); }

  /// \brief Get the length operand
  Value* length() const { return this->argument(2); }

  /// \brief Get the alignment of the destination, in bytes (0 if unspecified)
  uint64_t destination_alignment() const {
    return cast< IntegerConstant >(this->argument(3))->value().to< uint64_t >();
  }

  /// \brief Get the alignment of the source, in bytes (0 if unspecified)
  uint64_t source_alignment() const {
    return cast< IntegerConstant >(this->argument(4))->value().to< uint64_t >();
  }

  /// \brief Return true if the source or destination is volatile
  bool is_volatile() const {
    return cast< IntegerConstant >(this->argument(5))->value() != 0;
  }

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Statement* s) {
    return isa< IntrinsicCall >(s) &&
           cast< IntrinsicCall >(s)->intrinsic_id() == Intrinsic::MemoryMove;
  }

}; // end class MemoryMove

/// \brief Memory set statement (memset)
class MemorySet final : public IntrinsicCall {
private:
  /// \brief Private constructor
  MemorySet(Bundle* bundle,
            Value* pointer,
            Value* value,
            Value* length,
            uint64_t alignment,
            bool is_volatile);

public:
  /// \brief Static constructor
  static std::unique_ptr< MemorySet > create(Bundle* bundle,
                                             Value* pointer,
                                             Value* value,
                                             Value* length,
                                             uint64_t alignment,
                                             bool is_volatile);

  /// \brief Get the pointer
  Value* pointer() const { return this->argument(0); }

  /// \brief Get the value
  Value* value() const { return this->argument(1); }

  /// \brief Get the length
  Value* length() const { return this->argument(2); }

  /// \brief Get the alignment of the memory access, in bytes (0 if unspecified)
  uint64_t alignment() const {
    return cast< IntegerConstant >(this->argument(3))->value().to< uint64_t >();
  }

  /// \brief Return true if this statement has an alignment constraint
  bool has_alignment() const { return this->alignment() > 0; }

  /// \brief Return true if the destination is volatile
  bool is_volatile() const {
    return cast< IntegerConstant >(this->argument(4))->value() != 0;
  }

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Statement* s) {
    return isa< IntrinsicCall >(s) &&
           cast< IntrinsicCall >(s)->intrinsic_id() == Intrinsic::MemorySet;
  }

}; // end class MemorySet

/// \brief Variable argument start statement (va_start)
class VarArgStart final : public IntrinsicCall {
private:
  /// \brief Private constructor
  VarArgStart(Bundle* bundle, Value* operand);

public:
  /// \brief Static constructor
  static std::unique_ptr< VarArgStart > create(Bundle* bundle, Value* operand);

  /// \brief Get the operand
  Value* operand() const { return this->argument(0); }

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Statement* s) {
    return isa< IntrinsicCall >(s) &&
           cast< IntrinsicCall >(s)->intrinsic_id() == Intrinsic::VarArgStart;
  }

}; // end class VarArgStart

/// \brief Variable argument end statement (va_end)
class VarArgEnd final : public IntrinsicCall {
private:
  /// \brief Private constructor
  VarArgEnd(Bundle* bundle, Value* operand);

public:
  /// \brief Static constructor
  static std::unique_ptr< VarArgEnd > create(Bundle* bundle, Value* operand);

  /// \brief Get the operand
  Value* operand() const { return this->argument(0); }

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Statement* s) {
    return isa< IntrinsicCall >(s) &&
           cast< IntrinsicCall >(s)->intrinsic_id() == Intrinsic::VarArgEnd;
  }

}; // end class VarArgEnd

/// \brief Variable argument get statement (va_arg)
class VarArgGet final : public IntrinsicCall {
private:
  /// \brief Private constructor
  VarArgGet(Bundle* bundle, InternalVariable* result, Value* operand);

public:
  /// \brief Static constructor
  static std::unique_ptr< VarArgGet > create(Bundle* bundle,
                                             InternalVariable* result,
                                             Value* operand);

  /// \brief Get the operand
  Value* operand() const { return this->argument(0); }

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Statement* s) {
    return isa< IntrinsicCall >(s) &&
           cast< IntrinsicCall >(s)->intrinsic_id() == Intrinsic::VarArgGet;
  }

}; // end class VarArgGet

/// \brief Variable argument copy statement (va_copy)
class VarArgCopy final : public IntrinsicCall {
private:
  /// \brief Private constructor
  VarArgCopy(Bundle* bundle, Value* destination, Value* source);

public:
  /// \brief Static constructor
  static std::unique_ptr< VarArgCopy > create(Bundle* bundle,
                                              Value* destination,
                                              Value* source);

  /// \brief Get the destination operand
  Value* destination() const { return this->argument(0); }

  /// \brief Get the source operand
  Value* source() const { return this->argument(1); }

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Statement* s) {
    return isa< IntrinsicCall >(s) &&
           cast< IntrinsicCall >(s)->intrinsic_id() == Intrinsic::VarArgCopy;
  }

}; // end class VarArgCopy

/// \brief Save the current state of the function stack
class StackSave final : public IntrinsicCall {
private:
  /// \brief Private constructor
  StackSave(Bundle* bundle, InternalVariable* result);

public:
  /// \brief Static constructor
  static std::unique_ptr< StackSave > create(Bundle* bundle,
                                             InternalVariable* result);

  /// \brief Get the result variable
  InternalVariable* result() const {
    return cast< InternalVariable >(this->_result);
  }

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Statement* s) {
    return isa< IntrinsicCall >(s) &&
           cast< IntrinsicCall >(s)->intrinsic_id() == Intrinsic::StackSave;
  }

}; // end class StackSave

/// \brief Restore the state of the function stack corresponding to a stacksave
class StackRestore final : public IntrinsicCall {
private:
  /// \brief Private constructor
  StackRestore(Bundle* bundle, Value* operand);

public:
  /// \brief Static constructor
  static std::unique_ptr< StackRestore > create(Bundle* bundle, Value* operand);

  /// \brief Get the pointer operand
  Value* operand() const { return this->argument(0); }

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Statement* s) {
    return isa< IntrinsicCall >(s) &&
           cast< IntrinsicCall >(s)->intrinsic_id() == Intrinsic::StackRestore;
  }

}; // end class StackRestore

/// \brief Landing pad statement
///
// The landingpad instruction holds all of the information necessary to generate
// correct exception handling.
class LandingPad final : public Statement {
private:
  /// \brief Private constructor
  explicit LandingPad(InternalVariable* result);

public:
  /// \brief Static constructor
  static std::unique_ptr< LandingPad > create(InternalVariable* result);

  /// \brief Get the result variable
  InternalVariable* result() const {
    return cast< InternalVariable >(this->_result);
  }

  /// \brief Dump the statement for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Clone the statement
  std::unique_ptr< Statement > clone() const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Statement* s) {
    return s->kind() == LandingPadKind;
  }

}; // end class LandingPad

/// \brief Resume statement
class Resume final : public Statement {
private:
  /// \brief Private constructor
  explicit Resume(InternalVariable* operand);

public:
  /// \brief Static constructor
  static std::unique_ptr< Resume > create(InternalVariable* operand);

  /// \brief Get the operand
  InternalVariable* operand() const {
    return cast< InternalVariable >(this->_operands[0]);
  }

  /// \brief Dump the statement for debugging purpose
  void dump(std::ostream&) const override;

  /// \brief Clone the statement
  std::unique_ptr< Statement > clone() const override;

  /// \brief Method for type support (isa, cast, dyn_cast)
  static bool classof(const Statement* s) { return s->kind() == ResumeKind; }

}; // end class Resume

} // end namespace ar
} // end namespace ikos
