/*******************************************************************************
 *
 * \file
 * \brief Implementation of the type checker
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

#include <ikos/ar/format/namer.hpp>
#include <ikos/ar/format/text.hpp>
#include <ikos/ar/semantic/statement_visitor.hpp>
#include <ikos/ar/support/assert.hpp>
#include <ikos/ar/verify/type.hpp>

namespace ikos {
namespace ar {

// Warning: it is very important to write:
// valid = f(err, ...) && valid;
// And not:
// valid = valid && f(err, ...);
//
// This is because f(err, ...) can have side effects (typically, writes to err),
// and with the second pattern, the compiler will remove the call to f() if
// valid is false because of short-circuiting.

bool TypeVerifier::verify(Bundle* bundle, std::ostream& err) const {
  bool valid = true;
  for (auto it = bundle->global_begin(), et = bundle->global_end();
       it != et && (this->_all || valid);
       ++it) {
    valid = this->verify(*it, err) && valid;
  }
  for (auto it = bundle->function_begin(), et = bundle->function_end();
       it != et && (this->_all || valid);
       ++it) {
    valid = this->verify(*it, err) && valid;
  }
  return valid;
}

bool TypeVerifier::verify(GlobalVariable* gv, std::ostream& err) const {
  return gv->is_declaration() || this->verify(gv->initializer(), err, nullptr);
}

bool TypeVerifier::verify(Function* f, std::ostream& err) const {
  if (f->is_declaration()) {
    return true;
  }

  bool valid = true;
  FunctionType* type = f->type();

  // check var arg
  if ((this->_all || valid) && f->is_var_arg() != type->is_var_arg()) {
    valid = false;
    err << "error: function '" << f->name() << "' ";
    err << (f->is_var_arg() ? "is marked" : "is not marked");
    err << " var-arg but its type ";
    err << (type->is_var_arg() ? "is marked" : "is not marked");
    err << " var-arg\n";
  }

  // check the number of parameters
  if ((this->_all || valid) && f->num_parameters() != type->num_parameters()) {
    valid = false;
    err << "error: number of parameters of function '" << f->name()
        << "' does not match its type (" << f->num_parameters()
        << " != " << type->num_parameters() << ")\n";
  }

  // check each parameter
  auto fun_param_it = f->param_begin();
  auto fun_param_et = f->param_end();
  auto type_param_it = type->param_begin();
  auto type_param_et = type->param_end();
  while (fun_param_it != fun_param_et && type_param_it != type_param_et &&
         (this->_all || valid)) {
    if ((*fun_param_it)->type() != *type_param_it) {
      valid = false;
      err << "error: type of parameter ";
      TextFormatter().format(err, *fun_param_it, Namer(f->body()));
      err << " of function '" << f->name()
          << "' does not match the function type (";
      TextFormatter().format(err, (*fun_param_it)->type());
      err << " != ";
      TextFormatter().format(err, (*type_param_it));
      err << ")\n";
    }
    ++fun_param_it;
    ++type_param_it;
  }

  // check the function body
  if (this->_all || valid) {
    valid = this->verify(f->body(), err, type->return_type()) && valid;
  }

  return valid;
}

bool TypeVerifier::verify(Code* code,
                          std::ostream& err,
                          Type* return_type) const {
  bool valid = true;
  for (auto it = code->begin(), et = code->end();
       it != et && (this->_all || valid);
       ++it) {
    valid = this->verify(*it, err, return_type) && valid;
  }
  return valid;
}

/// \brief Type checker for statements
class StatementTypeVerifier {
public:
  using ResultType = bool;

public:
  std::ostream& err;
  Type* return_type;

public:
  StatementTypeVerifier(std::ostream& err_, Type* return_type_)
      : err(err_), return_type(return_type_) {}

private:
  //
  // Helpers for type checking
  //

  /// \brief Check that the given type is an unsigned integer type
  bool check_unsigned_int(Statement* s, Type* ty, const char* desc) {
    if (!ty->is_unsigned_integer()) {
      err << "error: " << desc << " of statement '";
      TextFormatter().format(err, s);
      err << "' is not an unsigned integer\n";
      return false;
    }
    return true;
  }

  /// \brief Check that the given type is a signed integer type
  bool check_signed_int(Statement* s, Type* ty, const char* desc) {
    if (!ty->is_signed_integer()) {
      err << "error: " << desc << " of statement '";
      TextFormatter().format(err, s);
      err << "' is not a signed integer\n";
      return false;
    }
    return true;
  }

  /// \brief Check that the given type is a floating point type
  bool check_float(Statement* s, Type* ty, const char* desc) {
    if (!ty->is_float()) {
      err << "error: " << desc << " of statement '";
      TextFormatter().format(err, s);
      err << "' is not a floating point\n";
      return false;
    }
    return true;
  }

  /// \brief Check that the given type is a pointer type
  bool check_pointer(Statement* s, Type* ty, const char* desc) {
    if (!ty->is_pointer()) {
      err << "error: " << desc << " of statement '";
      TextFormatter().format(err, s);
      err << "' is not a pointer\n";
      return false;
    }
    return true;
  }

  /// \brief Check that the given type is an aggregate type
  bool check_aggregate(Statement* s, Type* ty, const char* desc) {
    if (!ty->is_aggregate()) {
      err << "error: " << desc << " of statement '";
      TextFormatter().format(err, s);
      err << "' is not an aggregate\n";
      return false;
    }
    return true;
  }

  /// \brief Check that the given type is a vector type
  bool check_vector(Statement* s, Type* ty, const char* desc) {
    if (!ty->is_vector()) {
      err << "error: " << desc << " of statement '";
      TextFormatter().format(err, s);
      err << "' is not a vector\n";
      return false;
    }
    return true;
  }

  /// \brief Check that the given types are equals
  bool check_type_match(Statement* s, Type* t1, Type* t2) {
    if (t1 != t2) {
      err << "error: type mismatch in statement '";
      TextFormatter().format(err, s);
      err << "' (";
      TextFormatter().format(err, t1);
      err << " != ";
      TextFormatter().format(err, t2);
      err << ")\n";
      return false;
    }
    return true;
  }

  /// \brief Check that the given types are equals
  bool check_equals(Statement* s, Type* t1, Type* t2, const char* desc) {
    if (t1 != t2) {
      err << "error: " << desc << " of statement '";
      TextFormatter().format(err, s);
      err << "' has type ";
      TextFormatter().format(err, t1);
      err << ", was expecting type ";
      TextFormatter().format(err, t2);
      err << "\n";
      return false;
    }
    return true;
  }

  /// \brief Check that the given type is an exception structure type
  ///
  /// Checks that the type is equivalent to {ui8*, ui32}
  bool check_exception_struct(Statement* s, Type* ty, const char* desc) {
    if (ty->is_struct()) {
      auto sty = cast< StructType >(ty);
      if (sty->num_fields() == 2) {
        auto it = sty->field_begin();
        Type* first_ty = it->type;
        ++it;
        Type* second_ty = it->type;
        if (first_ty == PointerType::get(s->context(),
                                         IntegerType::si8(s->context())) &&
            second_ty == IntegerType::si32(s->context())) {
          return true;
        }
      }
    }
    err << "error: " << desc << " of statement '";
    TextFormatter().format(err, s);
    err << "' is not an exception structure\n";
    return false;
  }

public:
  //
  // visit statements
  //

  bool operator()(Assignment* s) {
    return this->check_type_match(s, s->result()->type(), s->operand()->type());
  }

  bool operator()(UnaryOperation* s) {
    Type* result_ty = s->result()->type();
    Type* operand_ty = s->operand()->type();

    switch (s->op()) {
      case UnaryOperation::UTrunc: {
        if (!this->check_unsigned_int(s, result_ty, "result") ||
            !this->check_unsigned_int(s, operand_ty, "operand")) {
          return false;
        }
        auto int_result_ty = cast< IntegerType >(result_ty);
        auto int_operand_ty = cast< IntegerType >(operand_ty);
        if (int_result_ty->bit_width() >= int_operand_ty->bit_width()) {
          err << "error: the result type of statement '";
          TextFormatter().format(err, s);
          err << "' is bigger than the operand type ("
              << int_result_ty->bit_width()
              << " >= " << int_operand_ty->bit_width() << ")\n";
          return false;
        }
        return true;
      }
      case UnaryOperation::STrunc: {
        if (!this->check_signed_int(s, result_ty, "result") ||
            !this->check_signed_int(s, operand_ty, "operand")) {
          return false;
        }
        auto int_result_ty = cast< IntegerType >(result_ty);
        auto int_operand_ty = cast< IntegerType >(operand_ty);
        if (int_result_ty->bit_width() >= int_operand_ty->bit_width()) {
          err << "error: the result type of statement '";
          TextFormatter().format(err, s);
          err << "' is bigger than the operand type ("
              << int_result_ty->bit_width()
              << " >= " << int_operand_ty->bit_width() << ")\n";
          return false;
        }
        return true;
      }
      case UnaryOperation::ZExt: {
        if (!this->check_unsigned_int(s, result_ty, "result") ||
            !this->check_unsigned_int(s, operand_ty, "operand")) {
          return false;
        }
        auto int_result_ty = cast< IntegerType >(result_ty);
        auto int_operand_ty = cast< IntegerType >(operand_ty);
        if (int_result_ty->bit_width() <= int_operand_ty->bit_width()) {
          err << "error: the result type of statement '";
          TextFormatter().format(err, s);
          err << "' is smaller than the operand type ("
              << int_result_ty->bit_width()
              << " <= " << int_operand_ty->bit_width() << ")\n";
          return false;
        }
        return true;
      }
      case UnaryOperation::SExt: {
        if (!this->check_signed_int(s, result_ty, "result") ||
            !this->check_signed_int(s, operand_ty, "operand")) {
          return false;
        }
        auto int_result_ty = cast< IntegerType >(result_ty);
        auto int_operand_ty = cast< IntegerType >(operand_ty);
        if (int_result_ty->bit_width() <= int_operand_ty->bit_width()) {
          err << "error: the result type of statement '";
          TextFormatter().format(err, s);
          err << "' is smaller than the operand type ("
              << int_result_ty->bit_width()
              << " <= " << int_operand_ty->bit_width() << ")\n";
          return false;
        }
        return true;
      }
      case UnaryOperation::FPTrunc: {
        if (!this->check_float(s, result_ty, "result") ||
            !this->check_float(s, operand_ty, "operand")) {
          return false;
        }
        auto float_result_ty = cast< FloatType >(result_ty);
        auto float_operand_ty = cast< FloatType >(operand_ty);
        if (float_result_ty->bit_width() >= float_operand_ty->bit_width()) {
          err << "error: the result type of statement '";
          TextFormatter().format(err, s);
          err << "' is bigger than the operand type ("
              << float_result_ty->bit_width()
              << " >= " << float_operand_ty->bit_width() << ")\n";
          return false;
        }
        return true;
      }
      case UnaryOperation::FPExt: {
        if (!this->check_float(s, result_ty, "result") ||
            !this->check_float(s, operand_ty, "operand")) {
          return false;
        }
        auto float_result_ty = cast< FloatType >(result_ty);
        auto float_operand_ty = cast< FloatType >(operand_ty);
        if (float_result_ty->bit_width() <= float_operand_ty->bit_width()) {
          err << "error: the result type of statement '";
          TextFormatter().format(err, s);
          err << "' is smaller than the operand type ("
              << float_result_ty->bit_width()
              << " <= " << float_operand_ty->bit_width() << ")\n";
          return false;
        }
        return true;
      }
      case UnaryOperation::FPToUI: {
        // note: the bit-width might be different
        return this->check_unsigned_int(s, result_ty, "result") &&
               this->check_float(s, operand_ty, "operand");
      }
      case UnaryOperation::FPToSI: {
        // note: the bit-width might be different
        return this->check_signed_int(s, result_ty, "result") &&
               this->check_float(s, operand_ty, "operand");
      }
      case UnaryOperation::UIToFP: {
        // note: the bit-width might be different
        return this->check_float(s, result_ty, "result") &&
               this->check_unsigned_int(s, operand_ty, "operand");
      }
      case UnaryOperation::SIToFP: {
        // note: the bit-width might be different
        return this->check_float(s, result_ty, "result") &&
               this->check_signed_int(s, operand_ty, "operand");
      }
      case UnaryOperation::PtrToUI: {
        // note: the bit-width might be different
        return this->check_unsigned_int(s, result_ty, "result") &&
               this->check_pointer(s, operand_ty, "operand");
      }
      case UnaryOperation::PtrToSI: {
        // note: the bit-width might be different
        return this->check_signed_int(s, result_ty, "result") &&
               this->check_pointer(s, operand_ty, "operand");
      }
      case UnaryOperation::UIToPtr: {
        // note: the bit-width might be different
        return this->check_pointer(s, result_ty, "result") &&
               this->check_unsigned_int(s, operand_ty, "operand");
      }
      case UnaryOperation::SIToPtr: {
        // note: the bit-width might be different
        return this->check_pointer(s, result_ty, "result") &&
               this->check_signed_int(s, operand_ty, "operand");
      }
      case UnaryOperation::Bitcast: {
        // Valid bitcasts are:
        //   * pointer casts: A* to B*
        //   * primitive type casts with the same bit-width
        //
        // A primitive type is either an integer, a floating point or a vector
        // of integers or floating points.
        if ((result_ty->is_pointer() && operand_ty->is_pointer()) ||
            (result_ty->is_primitive() && operand_ty->is_primitive() &&
             result_ty->primitive_bit_width() ==
                 operand_ty->primitive_bit_width())) {
          return true;
        } else {
          err << "error: invalid bitcast '";
          TextFormatter().format(err, s);
          err << "' from ";
          TextFormatter().format(err, operand_ty);
          err << " to ";
          TextFormatter().format(err, result_ty);
          err << "\n";
          return false;
        }
      }
      default: {
        ikos_unreachable("unexpected operator");
      }
    }
  }

  bool operator()(BinaryOperation* s) {
    Type* result_ty = s->result()->type();
    Type* left_ty = s->left()->type();
    Type* right_ty = s->right()->type();

    if (!this->check_type_match(s, result_ty, left_ty) ||
        !this->check_type_match(s, result_ty, right_ty)) {
      return false;
    }

    // Allow vector types
    if (result_ty->is_vector()) {
      result_ty = cast< VectorType >(result_ty)->element_type();
    }

    if (s->is_unsigned_op()) {
      return this->check_unsigned_int(s, result_ty, "result");
    } else if (s->is_signed_op()) {
      return this->check_signed_int(s, result_ty, "result");
    } else if (s->is_float_op()) {
      return this->check_float(s, result_ty, "result");
    } else {
      ikos_unreachable("unexpected operator");
    }
  }

  bool operator()(Comparison* s) {
    Type* left_ty = s->left()->type();
    Type* right_ty = s->right()->type();

    if (s->is_unsigned_predicate()) {
      return this->check_unsigned_int(s, left_ty, "left operand") &&
             this->check_unsigned_int(s, right_ty, "right operand") &&
             this->check_type_match(s, left_ty, right_ty);
    } else if (s->is_signed_predicate()) {
      return this->check_signed_int(s, left_ty, "left operand") &&
             this->check_signed_int(s, right_ty, "right operand") &&
             this->check_type_match(s, left_ty, right_ty);
    } else if (s->is_float_predicate()) {
      return this->check_float(s, left_ty, "left operand") &&
             this->check_float(s, right_ty, "right operand") &&
             this->check_type_match(s, left_ty, right_ty);
    } else if (s->is_pointer_predicate()) {
      // note: the type of the operands might be different
      return this->check_pointer(s, left_ty, "left operand") &&
             this->check_pointer(s, right_ty, "right operand");
    } else {
      ikos_unreachable("unexpected operator");
    }
  }

  bool operator()(ReturnValue* s) {
    if (return_type->is_void()) {
      if (s->has_operand()) {
        err << "error: statement '";
        TextFormatter().format(err, s);
        err << "' was not expecting an operand\n";
        return false;
      } else {
        return true;
      }
    } else {
      if (s->has_operand()) {
        if (return_type != s->operand()->type()) {
          err << "error: statement '";
          TextFormatter().format(err, s);
          err << "' returns a ";
          TextFormatter().format(err, s->operand()->type());
          err << " but function return type is ";
          TextFormatter().format(err, return_type);
          err << "\n";
          return false;
        } else {
          return true;
        }
      } else {
        err << "error: statement '";
        TextFormatter().format(err, s);
        err << "', was expecting an operand of type ";
        TextFormatter().format(err, return_type);
        err << "\n";
        return false;
      }
    }
  }

  bool operator()(Unreachable* /*unreachable*/) { return true; }

  bool operator()(Allocate* s) {
    return this->check_equals(s,
                              s->array_size()->type(),
                              IntegerType::size_type(s->bundle()),
                              "array size operand");
  }

  bool operator()(PointerShift* s) {
    if (!this->check_pointer(s, s->result()->type(), "result") ||
        !this->check_pointer(s, s->pointer()->type(), "first operand")) {
      return false;
    }

    // check all operands
    std::size_t n = 2;
    IntegerType* size_type = IntegerType::size_type(s->bundle());
    for (auto it = s->term_begin(), et = s->term_end(); it != et; ++it, ++n) {
      Value* op = (*it).second;
      // accept integers of any bit-width and sign
      if (!op->type()->is_integer()) {
        err << "error: operand " << n << " of statement '";
        TextFormatter().format(err, s);
        err << "' is not an integer\n";
        return false;
      }
      MachineInt factor = (*it).first;
      if (factor.bit_width() != size_type->bit_width() ||
          factor.sign() != size_type->sign()) {
        err << "error: factor " << n << " of statement '";
        TextFormatter().format(err, s);
        err << "' is not a ";
        TextFormatter().format(err, size_type);
        err << "\n";
        return false;
      }
    }

    return true;
  }

  bool operator()(Load* s) {
    Type* result_ty = s->result()->type();
    Type* operand_ty = s->operand()->type();
    if (!this->check_pointer(s, operand_ty, "operand")) {
      return false;
    }
    auto ptr_operand_ty = cast< PointerType >(operand_ty);
    return this->check_equals(s,
                              result_ty,
                              ptr_operand_ty->pointee(),
                              "result");
  }

  bool operator()(Store* s) {
    Type* pointer_ty = s->pointer()->type();
    Type* value_ty = s->value()->type();
    if (!this->check_pointer(s, pointer_ty, "first operand")) {
      return false;
    }
    auto ptr_pointer_ty = cast< PointerType >(pointer_ty);
    return this->check_equals(s,
                              value_ty,
                              ptr_pointer_ty->pointee(),
                              "second operand");
  }

  bool operator()(ExtractElement* s) {
    return this->check_aggregate(s,
                                 s->aggregate()->type(),
                                 "aggregate operand") &&
           this->check_equals(s,
                              s->offset()->type(),
                              IntegerType::size_type(s->bundle()),
                              "offset operand");
  }

  bool operator()(InsertElement* s) {
    return this->check_aggregate(s,
                                 s->aggregate()->type(),
                                 "aggregate operand") &&
           this->check_equals(s,
                              s->offset()->type(),
                              IntegerType::size_type(s->bundle()),
                              "offset operand");
  }

  bool operator()(ShuffleVector* s) {
    return (this->check_vector(s, s->left()->type(), "left operand") &&
            this->check_vector(s, s->right()->type(), "right operand") &&
            this->check_type_match(s, s->result()->type(), s->left()->type()) &&
            this->check_type_match(s, s->result()->type(), s->right()->type()));
  }

  bool operator()(CallBase* s) {
    // Check called value
    Type* called_ty = s->called()->type();

    if (!this->check_pointer(s, called_ty, "called value")) {
      return false;
    }

    auto ptr_called_ty = cast< PointerType >(called_ty);

    if (!ptr_called_ty->pointee()->is_function()) {
      err << "error: called value of statement '";
      TextFormatter().format(err, s);
      err << "' is not a function pointer\n";
      return false;
    }

    auto fun_ty = cast< FunctionType >(ptr_called_ty->pointee());

    // Check the result type
    if (isa< FunctionPointerConstant >(s->called()) &&
        cast< FunctionPointerConstant >(s->called())
                ->function()
                ->intrinsic_id() == Intrinsic::VarArgGet) {
      // except for VarArgGet, do nothing
    } else if (fun_ty->return_type()->is_void() && s->has_result()) {
      err << "error: called value of statement '";
      TextFormatter().format(err, s);
      err << "' has return type void, but the statement has a result\n";
      return false;
    } else if (!fun_ty->return_type()->is_void() && s->has_result() &&
               fun_ty->return_type() != s->result()->type()) {
      err << "error: called value of statement '";
      TextFormatter().format(err, s);
      err << "' has return type ";
      TextFormatter().format(err, fun_ty->return_type());
      err << " but the statement has a result of type";
      TextFormatter().format(err, s->result()->type());
      err << "\n";
      return false;
    }

    // Check the number of parameters
    if (fun_ty->is_var_arg() && s->num_arguments() < fun_ty->num_parameters()) {
      err << "error: number of parameters of function call '";
      TextFormatter().format(err, s);
      err << "' is smaller than the function type (" << s->num_arguments()
          << " < " << fun_ty->num_parameters() << ")\n";
      return false;
    } else if (!fun_ty->is_var_arg() &&
               s->num_arguments() != fun_ty->num_parameters()) {
      err << "error: number of parameters of function call '";
      TextFormatter().format(err, s);
      err << "' does not match the function type (" << s->num_arguments()
          << " != " << fun_ty->num_parameters() << ")\n";
      return false;
    }

    // Allow implicit casts if this is a call on a function pointer, or inline
    // assembly
    bool allow_implicit_bitcast =
        !isa< FunctionPointerConstant >(s->called()) ||
        isa< InlineAssemblyConstant >(s->called());

    // Check each parameter
    auto call_arg_it = s->arg_begin();
    auto call_arg_et = s->arg_end();
    auto ty_param_it = fun_ty->param_begin();
    auto ty_param_et = fun_ty->param_end();
    while (call_arg_it != call_arg_et && ty_param_it != ty_param_et) {
      Type* call_arg_type = (*call_arg_it)->type();
      Type* fun_param_type = *ty_param_it;

      if (call_arg_type != fun_param_type &&
          !(allow_implicit_bitcast &&
            TypeVerifier::is_implicit_bitcast(call_arg_type, fun_param_type))) {
        err << "error: type of parameter ";
        Namer namer(s->code());
        TextFormatter().format(err, *call_arg_it, namer);
        err << " of function call '";
        TextFormatter().format(err, s);
        err << "' does not match the function type (";
        TextFormatter().format(err, call_arg_type);
        err << " != ";
        TextFormatter().format(err, fun_param_type);
        err << ")\n";
        return false;
      }

      ++call_arg_it;
      ++ty_param_it;
    }

    return true;
  }

  bool operator()(Call* s) { return this->operator()(cast< CallBase >(s)); }

  bool operator()(Invoke* s) {
    if (s->normal_dest() == s->exception_dest()) {
      err << "error: statement '";
      TextFormatter().format(err, s);
      err << "' has the same normal and exception destination\n";
      return false;
    }

    return this->operator()(cast< CallBase >(s));
  }

  bool operator()(LandingPad* s) {
    return this->check_exception_struct(s, s->result()->type(), "result");
  }

  bool operator()(Resume* s) {
    return this->check_exception_struct(s, s->operand()->type(), "operand");
  }

}; // end class StatementTypeVerifier

bool TypeVerifier::verify(BasicBlock* bb,
                          std::ostream& err,
                          Type* return_type) const {
  StatementTypeVerifier vis(err, return_type);
  bool valid = true;
  for (auto it = bb->begin(), et = bb->end(); it != et && (this->_all || valid);
       ++it) {
    valid = apply_visitor(vis, *it) && valid;
  }
  return valid;
}

bool TypeVerifier::is_valid_call(ar::CallBase* call, ar::FunctionType* fun_ty) {
  // Check the result type
  if (fun_ty->return_type()->is_void() && call->has_result()) {
    return false;
  } else if (!fun_ty->return_type()->is_void() && call->has_result() &&
             call->result()->type() != fun_ty->return_type()) {
    return false;
  }

  // Check the number of parameters
  if (fun_ty->is_var_arg() &&
      call->num_arguments() < fun_ty->num_parameters()) {
    return false;
  } else if (!fun_ty->is_var_arg() &&
             call->num_arguments() != fun_ty->num_parameters()) {
    return false;
  }

  // Check each parameter
  auto call_arg_it = call->arg_begin();
  auto call_arg_et = call->arg_end();
  auto ty_param_it = fun_ty->param_begin();
  auto ty_param_et = fun_ty->param_end();
  while (call_arg_it != call_arg_et && ty_param_it != ty_param_et) {
    Type* call_arg_type = (*call_arg_it)->type();
    Type* fun_param_type = *ty_param_it;

    if (call_arg_type != fun_param_type &&
        !TypeVerifier::is_implicit_bitcast(call_arg_type, fun_param_type)) {
      return false;
    }

    ++call_arg_it;
    ++ty_param_it;
  }

  return true;
}

} // end namespace ar
} // end namespace ikos
