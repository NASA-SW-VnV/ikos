/*******************************************************************************
 *
 * \file
 * \brief Statement implementation
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
#include <ikos/ar/semantic/statement.hpp>

namespace ikos {
namespace ar {

// Statement

Statement::Statement(StatementKind kind, Variable* result, Operands operands)
    : _kind(kind),
      _parent(nullptr),
      _result(result),
      _operands(std::move(operands)) {}

Statement::Statement(StatementKind kind,
                     Variable* result,
                     std::initializer_list< Value* > operands)
    : _kind(kind), _parent(nullptr), _result(result), _operands(operands) {}

Statement::~Statement() = default;

BasicBlock::StatementIterator Statement::iterator() const {
  return std::find(this->parent()->begin(), this->parent()->end(), this);
}

Statement* Statement::prev_statement() const {
  auto it = this->iterator();
  if (it != this->parent()->begin()) {
    --it;
    return *it;
  } else {
    return nullptr;
  }
}

Statement* Statement::next_statement() const {
  auto it = this->iterator();
  ++it;
  if (it != this->parent()->end()) {
    return *it;
  } else {
    return nullptr;
  }
}

bool Statement::has_undefined_constant_operand() const {
  return std::any_of(this->_operands.begin(),
                     this->_operands.end(),
                     [](Value* op) { return isa< UndefinedConstant >(op); });
}

void Statement::set_parent(BasicBlock* parent) {
  this->_parent = parent;
}

// Assignment

Assignment::Assignment(InternalVariable* result, Value* operand)
    : Statement(AssignmentKind, result, {operand}) {
  ikos_assert_msg(result, "result is null");
  ikos_assert_msg(operand, "operand is null");
}

std::unique_ptr< Assignment > Assignment::create(InternalVariable* result,
                                                 Value* operand) {
  return std::unique_ptr< Assignment >(new Assignment(result, operand));
}

void Assignment::dump(std::ostream& o) const {
  this->result()->dump(o);
  o << " = ";
  this->operand()->dump(o);
}

std::unique_ptr< Statement > Assignment::clone() const {
  std::unique_ptr< Statement > stmt(
      new Assignment(this->result(), this->operand()));
  stmt->set_frontend(*this);
  return stmt;
}

// UnaryOperation

UnaryOperation::UnaryOperation(Operator op,
                               InternalVariable* result,
                               Value* operand)
    : Statement(UnaryOperationKind, result, {operand}), _op(op) {
  ikos_assert_msg(result, "result is null");
  ikos_assert_msg(operand, "operand is null");
}

std::unique_ptr< UnaryOperation > UnaryOperation::create(
    Operator op, InternalVariable* result, Value* operand) {
  return std::unique_ptr< UnaryOperation >(
      new UnaryOperation(op, result, operand));
}

std::string UnaryOperation::operator_text(Operator op) {
  switch (op) {
    case UTrunc:
      return "utrunc";
    case STrunc:
      return "strunc";
    case ZExt:
      return "zext";
    case SExt:
      return "sext";
    case FPTrunc:
      return "fptrunc";
    case FPExt:
      return "fpext";
    case FPToUI:
      return "fptoui";
    case FPToSI:
      return "fptosi";
    case UIToFP:
      return "uitofp";
    case SIToFP:
      return "sitofp";
    case PtrToUI:
      return "ptrtoui";
    case PtrToSI:
      return "ptrtosi";
    case UIToPtr:
      return "uitoptr";
    case SIToPtr:
      return "sitoptr";
    case Bitcast:
      return "bitcast";
    default:
      ikos_unreachable("unexpected operator");
  }
}

void UnaryOperation::dump(std::ostream& o) const {
  this->result()->dump(o);
  o << " = " << operator_text(this->_op) << " ";
  this->operand()->dump(o);
}

std::unique_ptr< Statement > UnaryOperation::clone() const {
  std::unique_ptr< Statement > stmt(
      new UnaryOperation(this->op(), this->result(), this->operand()));
  stmt->set_frontend(*this);
  return stmt;
}

// BinaryOperation

BinaryOperation::BinaryOperation(Operator op,
                                 InternalVariable* result,
                                 Value* left,
                                 Value* right,
                                 bool no_wrap,
                                 bool exact)
    : Statement(BinaryOperationKind, result, {left, right}),
      _op(op),
      _no_wrap(no_wrap),
      _exact(exact) {
  ikos_assert_msg(result, "result is null");
  ikos_assert_msg(left, "left is null");
  ikos_assert_msg(right, "right is null");
  ikos_assert(!no_wrap || is_wrapping_operator(op));
  ikos_assert(!exact || is_exact_operator(op));
}

std::unique_ptr< BinaryOperation > BinaryOperation::create(
    Operator op,
    InternalVariable* result,
    Value* left,
    Value* right,
    bool no_wrap,
    bool exact) {
  return std::unique_ptr< BinaryOperation >(
      new BinaryOperation(op, result, left, right, no_wrap, exact));
}

std::string BinaryOperation::operator_text(Operator op) {
  switch (op) {
    case UAdd:
      return "uadd";
    case USub:
      return "usub";
    case UMul:
      return "umul";
    case UDiv:
      return "udiv";
    case URem:
      return "urem";
    case UShl:
      return "ushl";
    case ULShr:
      return "ulshr";
    case UAShr:
      return "uashr";
    case UAnd:
      return "uand";
    case UOr:
      return "uor";
    case UXor:
      return "uxor";
    case SAdd:
      return "sadd";
    case SSub:
      return "ssub";
    case SMul:
      return "smul";
    case SDiv:
      return "sdiv";
    case SRem:
      return "srem";
    case SShl:
      return "sshl";
    case SLShr:
      return "slshr";
    case SAShr:
      return "sashr";
    case SAnd:
      return "sand";
    case SOr:
      return "sor";
    case SXor:
      return "sxor";
    case FAdd:
      return "fadd";
    case FSub:
      return "fsub";
    case FMul:
      return "fmul";
    case FDiv:
      return "fdiv";
    case FRem:
      return "frem";
    default:
      ikos_unreachable("unexpected operator");
  }
}

void BinaryOperation::dump(std::ostream& o) const {
  this->result()->dump(o);
  o << " = ";
  this->left()->dump(o);
  o << " ";
  o << operator_text(this->op());
  if (this->has_no_wrap()) {
    o << ".nw";
  }
  if (this->is_exact()) {
    o << ".exact";
  }
  o << " ";
  this->right()->dump(o);
}

std::unique_ptr< Statement > BinaryOperation::clone() const {
  std::unique_ptr< Statement > stmt(new BinaryOperation(this->op(),
                                                        this->result(),
                                                        this->left(),
                                                        this->right(),
                                                        this->has_no_wrap(),
                                                        this->is_exact()));
  stmt->set_frontend(*this);
  return stmt;
}

// Comparison

Comparison::Comparison(Predicate predicate, Value* left, Value* right)
    : Statement(ComparisonKind, nullptr, {left, right}), _predicate(predicate) {
  ikos_assert_msg(left, "left is null");
  ikos_assert_msg(right, "right is null");
}

std::unique_ptr< Comparison > Comparison::create(Predicate predicate,
                                                 Value* left,
                                                 Value* right) {
  return std::unique_ptr< Comparison >(new Comparison(predicate, left, right));
}

std::string Comparison::predicate_text(Predicate pred) {
  switch (pred) {
    case UIEQ:
      return "uieq";
    case UINE:
      return "uine";
    case UIGT:
      return "uigt";
    case UIGE:
      return "uige";
    case UILT:
      return "uilt";
    case UILE:
      return "uile";
    case SIEQ:
      return "sieq";
    case SINE:
      return "sine";
    case SIGT:
      return "sigt";
    case SIGE:
      return "sige";
    case SILT:
      return "silt";
    case SILE:
      return "sile";
    case FOEQ:
      return "foeq";
    case FOGT:
      return "fogt";
    case FOGE:
      return "foge";
    case FOLT:
      return "folt";
    case FOLE:
      return "fole";
    case FONE:
      return "fone";
    case FORD:
      return "ford";
    case FUNO:
      return "funo";
    case FUEQ:
      return "fueq";
    case FUGT:
      return "fugt";
    case FUGE:
      return "fuge";
    case FULT:
      return "fult";
    case FULE:
      return "fule";
    case FUNE:
      return "fune";
    case PEQ:
      return "peq";
    case PNE:
      return "pne";
    case PGT:
      return "pgt";
    case PGE:
      return "pge";
    case PLT:
      return "plt";
    case PLE:
      return "ple";
    default:
      ikos_unreachable("unexpected operator");
  }
}

Comparison::Predicate Comparison::inverse_predicate(Predicate pred) {
  switch (pred) {
    case UIEQ:
      return UINE;
    case UINE:
      return UIEQ;
    case UIGT:
      return UILE;
    case UIGE:
      return UILT;
    case UILT:
      return UIGE;
    case UILE:
      return UIGT;
    case SIEQ:
      return SINE;
    case SINE:
      return SIEQ;
    case SIGT:
      return SILE;
    case SIGE:
      return SILT;
    case SILT:
      return SIGE;
    case SILE:
      return SIGT;
    case FOEQ:
      return FUNE;
    case FOGT:
      return FULE;
    case FOGE:
      return FULT;
    case FOLT:
      return FUGE;
    case FOLE:
      return FUGT;
    case FONE:
      return FUEQ;
    case FORD:
      return FUNO;
    case FUNO:
      return FORD;
    case FUEQ:
      return FONE;
    case FUGT:
      return FOLE;
    case FUGE:
      return FOLT;
    case FULT:
      return FOGE;
    case FULE:
      return FOGT;
    case FUNE:
      return FOEQ;
    case PEQ:
      return PNE;
    case PNE:
      return PEQ;
    case PGT:
      return PLE;
    case PGE:
      return PLT;
    case PLT:
      return PGE;
    case PLE:
      return PGT;
    default:
      ikos_unreachable("unexpected operator");
  }
}

std::unique_ptr< Comparison > Comparison::inverse() const {
  std::unique_ptr< Comparison > stmt(
      new Comparison(inverse_predicate(this->predicate()),
                     this->left(),
                     this->right()));
  stmt->set_frontend(*this);
  return stmt;
}

void Comparison::dump(std::ostream& o) const {
  this->left()->dump(o);
  o << " " << predicate_text(this->predicate()) << " ";
  this->right()->dump(o);
}

std::unique_ptr< Statement > Comparison::clone() const {
  std::unique_ptr< Statement > stmt(
      new Comparison(this->predicate(), this->left(), this->right()));
  stmt->set_frontend(*this);
  return stmt;
}

// ReturnValue

ReturnValue::ReturnValue(Value* operand)
    : Statement(ReturnValueKind, nullptr, {}) {
  if (operand != nullptr) {
    this->_operands.push_back(operand);
  }
}

std::unique_ptr< ReturnValue > ReturnValue::create(Value* operand) {
  return std::unique_ptr< ReturnValue >(new ReturnValue(operand));
}

void ReturnValue::dump(std::ostream& o) const {
  if (this->has_operand()) {
    o << "return ";
    this->operand()->dump(o);
  } else {
    o << "return";
  }
}

std::unique_ptr< Statement > ReturnValue::clone() const {
  std::unique_ptr< Statement > stmt(new ReturnValue(this->operand_or_null()));
  stmt->set_frontend(*this);
  return stmt;
}

// Unreachable

Unreachable::Unreachable() : Statement(UnreachableKind, nullptr, {}) {}

std::unique_ptr< Unreachable > Unreachable::create() {
  return std::unique_ptr< Unreachable >(new Unreachable());
}

void Unreachable::dump(std::ostream& o) const {
  o << "unreachable";
}

std::unique_ptr< Statement > Unreachable::clone() const {
  std::unique_ptr< Statement > stmt(new Unreachable());
  stmt->set_frontend(*this);
  return stmt;
}

// Allocate

Allocate::Allocate(LocalVariable* result,
                   Type* allocated_type,
                   Value* array_size)
    : Statement(AllocateKind, result, {array_size}),
      _allocated_type(allocated_type) {
  ikos_assert_msg(result, "result is null");
  ikos_assert_msg(allocated_type, "allocated type is null");
  ikos_assert_msg(array_size, "array size is null");
}

std::unique_ptr< Allocate > Allocate::create(LocalVariable* result,
                                             Type* allocated_type,
                                             Value* array_size) {
  return std::unique_ptr< Allocate >(
      new Allocate(result, allocated_type, array_size));
}

void Allocate::dump(std::ostream& o) const {
  this->result()->dump(o);
  o << " = allocate ";
  this->allocated_type()->dump(o);
  o << ", ";
  this->array_size()->dump(o);
}

std::unique_ptr< Statement > Allocate::clone() const {
  std::unique_ptr< Statement > stmt(
      new Allocate(this->result(), this->allocated_type(), this->array_size()));
  stmt->set_frontend(*this);
  return stmt;
}

// PointerShift

PointerShift::PointerShift(InternalVariable* result,
                           Value* pointer,
                           const std::vector< Term >& terms)
    : Statement(PointerShiftKind, result, {pointer}) {
  ikos_assert_msg(result, "result is null");
  ikos_assert_msg(pointer, "pointer is null");
  ikos_assert_msg(!terms.empty(), "terms is empty");

  this->_factors.reserve(terms.size());
  this->_operands.reserve(1 + terms.size());

  for (const auto& term : terms) {
    this->_factors.push_back(term.first);
    this->_operands.push_back(term.second);
  }
}

PointerShift::PointerShift(InternalVariable* result,
                           Factors factors,
                           Operands operands)
    : Statement(PointerShiftKind, result, std::move(operands)),
      _factors(std::move(factors)) {}

std::unique_ptr< PointerShift > PointerShift::create(
    InternalVariable* result,
    Value* pointer,
    const std::vector< Term >& terms) {
  return std::unique_ptr< PointerShift >(
      new PointerShift(result, pointer, terms));
}

void PointerShift::dump(std::ostream& o) const {
  this->result()->dump(o);
  o << " = ptrshift ";
  this->pointer()->dump(o);
  for (auto it = this->term_begin(), et = this->term_end(); it != et; ++it) {
    auto term = *it;
    o << ", " << term.first << " * ";
    term.second->dump(o);
  }
}

std::unique_ptr< Statement > PointerShift::clone() const {
  std::unique_ptr< Statement > stmt(
      new PointerShift(this->result(), this->_factors, this->_operands));
  stmt->set_frontend(*this);
  return stmt;
}

// Load

Load::Load(InternalVariable* result,
           Value* operand,
           uint64_t alignment,
           bool is_volatile)
    : Statement(LoadKind, result, {operand}),
      _alignment(alignment),
      _is_volatile(is_volatile) {
  ikos_assert_msg(result, "result is null");
  ikos_assert_msg(operand, "operand is null");
}

std::unique_ptr< Load > Load::create(InternalVariable* result,
                                     Value* operand,
                                     uint64_t alignment,
                                     bool is_volatile) {
  return std::unique_ptr< Load >(
      new Load(result, operand, alignment, is_volatile));
}

void Load::dump(std::ostream& o) const {
  this->result()->dump(o);
  o << " = load ";
  if (this->is_volatile()) {
    o << "volatile ";
  }
  this->operand()->dump(o);
  if (this->has_alignment()) {
    o << ", align " << this->alignment();
  }
}

std::unique_ptr< Statement > Load::clone() const {
  std::unique_ptr< Statement > stmt(new Load(this->result(),
                                             this->operand(),
                                             this->alignment(),
                                             this->is_volatile()));
  stmt->set_frontend(*this);
  return stmt;
}

// Store

Store::Store(Value* pointer, Value* value, uint64_t alignment, bool is_volatile)
    : Statement(StoreKind, nullptr, {pointer, value}),
      _alignment(alignment),
      _is_volatile(is_volatile) {
  ikos_assert_msg(pointer, "pointer is null");
  ikos_assert_msg(value, "value is null");
}

std::unique_ptr< Store > Store::create(Value* pointer,
                                       Value* value,
                                       uint64_t alignment,
                                       bool is_volatile) {
  return std::unique_ptr< Store >(
      new Store(pointer, value, alignment, is_volatile));
}

void Store::dump(std::ostream& o) const {
  o << "store ";
  if (this->is_volatile()) {
    o << "volatile ";
  }
  this->pointer()->dump(o);
  o << ", ";
  this->value()->dump(o);
  if (this->has_alignment()) {
    o << ", align " << this->alignment();
  }
}

std::unique_ptr< Statement > Store::clone() const {
  std::unique_ptr< Statement > stmt(new Store(this->pointer(),
                                              this->value(),
                                              this->alignment(),
                                              this->is_volatile()));
  stmt->set_frontend(*this);
  return stmt;
}

// ExtractElement

ExtractElement::ExtractElement(InternalVariable* result,
                               Value* aggregate,
                               Value* offset)
    : Statement(ExtractElementKind, result, {aggregate, offset}) {
  ikos_assert_msg(result, "result is null");
  ikos_assert_msg(aggregate, "aggregate is null");
  ikos_assert_msg(offset, "offset is null");
}

std::unique_ptr< ExtractElement > ExtractElement::create(
    InternalVariable* result, Value* aggregate, Value* offset) {
  return std::unique_ptr< ExtractElement >(
      new ExtractElement(result, aggregate, offset));
}

void ExtractElement::dump(std::ostream& o) const {
  this->result()->dump(o);
  o << " = extractelement ";
  this->aggregate()->dump(o);
  o << ", ";
  this->offset()->dump(o);
}

std::unique_ptr< Statement > ExtractElement::clone() const {
  std::unique_ptr< Statement > stmt(
      new ExtractElement(this->result(), this->aggregate(), this->offset()));
  stmt->set_frontend(*this);
  return stmt;
}

// InsertElement

InsertElement::InsertElement(InternalVariable* result,
                             Value* aggregate,
                             Value* offset,
                             Value* element)
    : Statement(InsertElementKind, result, {aggregate, offset, element}) {
  ikos_assert_msg(result, "result is null");
  ikos_assert_msg(aggregate, "aggregate is null");
  ikos_assert_msg(offset, "offset is null");
  ikos_assert_msg(element, "element is null");
}

std::unique_ptr< InsertElement > InsertElement::create(InternalVariable* result,
                                                       Value* aggregate,
                                                       Value* offset,
                                                       Value* element) {
  return std::unique_ptr< InsertElement >(
      new InsertElement(result, aggregate, offset, element));
}

void InsertElement::dump(std::ostream& o) const {
  this->result()->dump(o);
  o << " = insertelement ";
  this->aggregate()->dump(o);
  o << ", ";
  this->offset()->dump(o);
  o << ", ";
  this->element()->dump(o);
}

std::unique_ptr< Statement > InsertElement::clone() const {
  std::unique_ptr< Statement > stmt(new InsertElement(this->result(),
                                                      this->aggregate(),
                                                      this->offset(),
                                                      this->element()));
  stmt->set_frontend(*this);
  return stmt;
}

// ShuffleVector

ShuffleVector::ShuffleVector(InternalVariable* result,
                             Value* left,
                             Value* right)
    : Statement(ShuffleVectorKind, result, {left, right}) {
  ikos_assert_msg(result, "result is null");
  ikos_assert_msg(left, "left is null");
  ikos_assert_msg(right, "right is null");
}

std::unique_ptr< ShuffleVector > ShuffleVector::create(InternalVariable* result,
                                                       Value* left,
                                                       Value* right) {
  return std::unique_ptr< ShuffleVector >(
      new ShuffleVector(result, left, right));
}

void ShuffleVector::dump(std::ostream& o) const {
  this->result()->dump(o);
  o << " = shufflevector ";
  this->left()->dump(o);
  o << ", ";
  this->right()->dump(o);
}

std::unique_ptr< Statement > ShuffleVector::clone() const {
  std::unique_ptr< Statement > stmt(
      new ShuffleVector(this->result(), this->left(), this->right()));
  stmt->set_frontend(*this);
  return stmt;
}

// CallBase

CallBase::CallBase(StatementKind kind,
                   InternalVariable* result,
                   Value* called,
                   const std::vector< Value* >& arguments)
    : Statement(kind, result, {called}) {
  ikos_assert_msg(called, "called is null");
  ikos_assert_msg(std::all_of(arguments.begin(),
                              arguments.end(),
                              [](Value* arg) { return arg; }),
                  "argument is null");

  this->_operands.reserve(1 + arguments.size());
  std::copy(arguments.begin(),
            arguments.end(),
            std::back_inserter(this->_operands));
}

CallBase::CallBase(StatementKind kind,
                   InternalVariable* result,
                   const Operands& operands)
    : Statement(kind, result, operands) {}

// Call

Call::Call(InternalVariable* result,
           Value* called,
           const std::vector< Value* >& arguments)
    : CallBase(CallKind, result, called, arguments) {}

Call::Call(InternalVariable* result, const Operands& operands)
    : CallBase(CallKind, result, operands) {}

std::unique_ptr< Call > Call::create(InternalVariable* result,
                                     Value* called,
                                     const std::vector< Value* >& arguments) {
  return std::unique_ptr< Call >(new Call(result, called, arguments));
}

std::unique_ptr< Call > Call::create(InternalVariable* result,
                                     Function* function,
                                     const std::vector< Value* >& arguments) {
  return std::unique_ptr< Call >(
      new Call(result, function->pointer(), arguments));
}

void Call::dump(std::ostream& o) const {
  if (this->has_result()) {
    this->result()->dump(o);
    o << " = ";
  }
  o << "call ";
  this->called()->dump(o);
  o << "(";
  for (auto it = this->arg_begin(), et = this->arg_end(); it != et;) {
    (*it)->dump(o);
    ++it;
    if (it != et) {
      o << ", ";
    }
  }
  o << ")";
}

std::unique_ptr< Statement > Call::clone() const {
  std::unique_ptr< Statement > stmt(
      new Call(this->result_or_null(), this->_operands));
  stmt->set_frontend(*this);
  return stmt;
}

// IntrinsicCall

IntrinsicCall::IntrinsicCall(InternalVariable* result,
                             Function* function,
                             const std::vector< Value* >& arguments)
    : Call(result, function->pointer(), arguments) {}

// Invoke

Invoke::Invoke(InternalVariable* result,
               Value* called,
               const std::vector< Value* >& arguments,
               BasicBlock* normal_dest,
               BasicBlock* exception_dest)
    : CallBase(InvokeKind, result, called, arguments),
      _normal_dest(normal_dest),
      _exception_dest(exception_dest) {
  ikos_assert_msg(normal_dest, "normal_dest is null");
  ikos_assert_msg(exception_dest, "exception_dest is null");
}

Invoke::Invoke(InternalVariable* result,
               const Operands& operands,
               BasicBlock* normal_dest,
               BasicBlock* exception_dest)
    : CallBase(InvokeKind, result, operands),
      _normal_dest(normal_dest),
      _exception_dest(exception_dest) {}

std::unique_ptr< Invoke > Invoke::create(InternalVariable* result,
                                         Value* called,
                                         const std::vector< Value* >& arguments,
                                         BasicBlock* normal_dest,
                                         BasicBlock* exception_dest) {
  return std::unique_ptr< Invoke >(
      new Invoke(result, called, arguments, normal_dest, exception_dest));
}

std::unique_ptr< Invoke > Invoke::create(InternalVariable* result,
                                         Function* function,
                                         const std::vector< Value* >& arguments,
                                         BasicBlock* normal_dest,
                                         BasicBlock* exception_dest) {
  return std::unique_ptr< Invoke >(new Invoke(result,
                                              function->pointer(),
                                              arguments,
                                              normal_dest,
                                              exception_dest));
}

void Invoke::dump(std::ostream& o) const {
  if (this->has_result()) {
    this->result()->dump(o);
    o << " = ";
  }
  o << "invoke ";
  this->called()->dump(o);
  o << "(";
  for (auto it = this->arg_begin(), et = this->arg_end(); it != et;) {
    (*it)->dump(o);
    ++it;
    if (it != et) {
      o << ", ";
    }
  }
  o << ")";
}

std::unique_ptr< Statement > Invoke::clone() const {
  std::unique_ptr< Statement > stmt(new Invoke(this->result_or_null(),
                                               this->_operands,
                                               this->normal_dest(),
                                               this->exception_dest()));
  stmt->set_frontend(*this);
  return stmt;
}

// Helper for alignment constant
static IntegerConstant* alignment_constant(Bundle* bundle, uint64_t value) {
  return IntegerConstant::get(bundle->context(),
                              IntegerType::ui32(bundle->context()),
                              MachineInt(value, 32, Unsigned));
}

// Helper for volatile constant
static IntegerConstant* volatile_constant(Bundle* bundle, bool value) {
  return IntegerConstant::get(bundle->context(),
                              IntegerType::ui1(bundle->context()),
                              value ? 1 : 0);
}

// MemoryCopy

MemoryCopy::MemoryCopy(Bundle* bundle,
                       Value* destination,
                       Value* source,
                       Value* length,
                       uint64_t destination_alignment,
                       uint64_t source_alignment,
                       bool is_volatile)
    : IntrinsicCall(nullptr,
                    bundle->intrinsic_function(Intrinsic::MemoryCopy),
                    {destination,
                     source,
                     length,
                     alignment_constant(bundle, destination_alignment),
                     alignment_constant(bundle, source_alignment),
                     volatile_constant(bundle, is_volatile)}) {
  ikos_assert_msg(destination, "destination is null");
  ikos_assert_msg(source, "source is null");
  ikos_assert_msg(length, "length is null");
}

std::unique_ptr< MemoryCopy > MemoryCopy::create(Bundle* bundle,
                                                 Value* destination,
                                                 Value* source,
                                                 Value* length,
                                                 uint64_t destination_alignment,
                                                 uint64_t source_alignment,
                                                 bool is_volatile) {
  return std::unique_ptr< MemoryCopy >(new MemoryCopy(bundle,
                                                      destination,
                                                      source,
                                                      length,
                                                      destination_alignment,
                                                      source_alignment,
                                                      is_volatile));
}

// MemoryMove

MemoryMove::MemoryMove(Bundle* bundle,
                       Value* destination,
                       Value* source,
                       Value* length,
                       uint64_t destination_alignment,
                       uint64_t source_alignment,
                       bool is_volatile)
    : IntrinsicCall(nullptr,
                    bundle->intrinsic_function(Intrinsic::MemoryMove),
                    {destination,
                     source,
                     length,
                     alignment_constant(bundle, destination_alignment),
                     alignment_constant(bundle, source_alignment),
                     volatile_constant(bundle, is_volatile)}) {
  ikos_assert_msg(destination, "destination is null");
  ikos_assert_msg(source, "source is null");
  ikos_assert_msg(length, "length is null");
}

std::unique_ptr< MemoryMove > MemoryMove::create(Bundle* bundle,
                                                 Value* destination,
                                                 Value* source,
                                                 Value* length,
                                                 uint64_t destination_alignment,
                                                 uint64_t source_alignment,
                                                 bool is_volatile) {
  return std::unique_ptr< MemoryMove >(new MemoryMove(bundle,
                                                      destination,
                                                      source,
                                                      length,
                                                      destination_alignment,
                                                      source_alignment,
                                                      is_volatile));
}

// MemorySet

MemorySet::MemorySet(Bundle* bundle,
                     Value* pointer,
                     Value* value,
                     Value* length,
                     uint64_t alignment,
                     bool is_volatile)
    : IntrinsicCall(nullptr,
                    bundle->intrinsic_function(Intrinsic::MemorySet),
                    {pointer,
                     value,
                     length,
                     alignment_constant(bundle, alignment),
                     volatile_constant(bundle, is_volatile)}) {
  ikos_assert_msg(pointer, "pointer is null");
  ikos_assert_msg(value, "value is null");
  ikos_assert_msg(length, "length is null");
}

std::unique_ptr< MemorySet > MemorySet::create(Bundle* bundle,
                                               Value* pointer,
                                               Value* value,
                                               Value* length,
                                               uint64_t alignment,
                                               bool is_volatile) {
  return std::unique_ptr< MemorySet >(
      new MemorySet(bundle, pointer, value, length, alignment, is_volatile));
}

// VarArgStart

VarArgStart::VarArgStart(Bundle* bundle, Value* operand)
    : IntrinsicCall(nullptr,
                    bundle->intrinsic_function(Intrinsic::VarArgStart),
                    {operand}) {
  ikos_assert_msg(operand, "operand is null");
}

std::unique_ptr< VarArgStart > VarArgStart::create(Bundle* bundle,
                                                   Value* operand) {
  return std::unique_ptr< VarArgStart >(new VarArgStart(bundle, operand));
}

// VarArgEnd

VarArgEnd::VarArgEnd(Bundle* bundle, Value* operand)
    : IntrinsicCall(nullptr,
                    bundle->intrinsic_function(Intrinsic::VarArgEnd),
                    {operand}) {
  ikos_assert_msg(operand, "operand is null");
}

std::unique_ptr< VarArgEnd > VarArgEnd::create(Bundle* bundle, Value* operand) {
  return std::unique_ptr< VarArgEnd >(new VarArgEnd(bundle, operand));
}

// VarArgGet

VarArgGet::VarArgGet(Bundle* bundle, InternalVariable* result, Value* operand)
    : IntrinsicCall(result,
                    bundle->intrinsic_function(Intrinsic::VarArgGet),
                    {operand}) {
  ikos_assert_msg(result, "result is null");
  ikos_assert_msg(operand, "operand is null");
}

std::unique_ptr< VarArgGet > VarArgGet::create(Bundle* bundle,
                                               InternalVariable* result,
                                               Value* operand) {
  return std::unique_ptr< VarArgGet >(new VarArgGet(bundle, result, operand));
}

// VarArgCopy

VarArgCopy::VarArgCopy(Bundle* bundle, Value* destination, Value* source)
    : IntrinsicCall(nullptr,
                    bundle->intrinsic_function(Intrinsic::VarArgCopy),
                    {destination, source}) {
  ikos_assert_msg(destination, "destination is null");
  ikos_assert_msg(source, "source is null");
}

std::unique_ptr< VarArgCopy > VarArgCopy::create(Bundle* bundle,
                                                 Value* destination,
                                                 Value* source) {
  return std::unique_ptr< VarArgCopy >(
      new VarArgCopy(bundle, destination, source));
}

// StackSave

StackSave::StackSave(Bundle* bundle, InternalVariable* result)
    : IntrinsicCall(result,
                    bundle->intrinsic_function(Intrinsic::StackSave),
                    {}) {
  ikos_assert_msg(result, "result is null");
}

std::unique_ptr< StackSave > StackSave::create(Bundle* bundle,
                                               InternalVariable* result) {
  return std::unique_ptr< StackSave >(new StackSave(bundle, result));
}

// StackRestore

StackRestore::StackRestore(Bundle* bundle, Value* operand)
    : IntrinsicCall(nullptr,
                    bundle->intrinsic_function(Intrinsic::StackRestore),
                    {operand}) {
  ikos_assert_msg(operand, "operand is null");
}

std::unique_ptr< StackRestore > StackRestore::create(Bundle* bundle,
                                                     Value* operand) {
  return std::unique_ptr< StackRestore >(new StackRestore(bundle, operand));
}

// LandingPad

LandingPad::LandingPad(InternalVariable* result)
    : Statement(LandingPadKind, result, {}) {
  ikos_assert_msg(result, "result is null");
}

std::unique_ptr< LandingPad > LandingPad::create(InternalVariable* result) {
  return std::unique_ptr< LandingPad >(new LandingPad(result));
}

void LandingPad::dump(std::ostream& o) const {
  this->result()->dump(o);
  o << " = landingpad";
}

std::unique_ptr< Statement > LandingPad::clone() const {
  std::unique_ptr< Statement > stmt(new LandingPad(this->result()));
  stmt->set_frontend(*this);
  return stmt;
}

// Resume

Resume::Resume(InternalVariable* operand)
    : Statement(ResumeKind, nullptr, {operand}) {
  ikos_assert_msg(operand, "operand is null");
}

std::unique_ptr< Resume > Resume::create(InternalVariable* operand) {
  return std::unique_ptr< Resume >(new Resume(operand));
}

void Resume::dump(std::ostream& o) const {
  o << "resume ";
  this->operand()->dump(o);
}

std::unique_ptr< Statement > Resume::clone() const {
  std::unique_ptr< Statement > stmt(new Resume(this->operand()));
  stmt->set_frontend(*this);
  return stmt;
}

} // end namespace ar
} // end namespace ikos
