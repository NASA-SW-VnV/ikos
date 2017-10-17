/*******************************************************************************
 *
 * Visitor methods for ARBOS classes.
 *
 * Authors: Nija Shi
 *          Arnaud J. Venet
 *
 * Contributors: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2017 United States Government as represented by the
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

#include <map>
#include <memory>
#include <sstream>
#include <string>

#include <arbos/semantics/ar.hpp>

using namespace arbos;

void AR_Integer_Comparison::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Left Operand");
  (*_left_op).accept(visitor);
  visitor->attributeEnd(*this, "Left Operand");
  visitor->attributeStart(*this, "Right Operand");
  (*_right_op).accept(visitor);
  visitor->attributeEnd(*this, "Right Operand");
  visitor->nodeEnd(*this);
}

void AR_FP_Comparison::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Left Operand");
  (*_left_op).accept(visitor);
  visitor->attributeEnd(*this, "Left Operand");
  visitor->attributeStart(*this, "Right Operand");
  (*_right_op).accept(visitor);
  visitor->attributeEnd(*this, "Right Operand");
  visitor->nodeEnd(*this);
}

void AR_Assignment::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Left Operand");
  (*_left_op).accept(visitor);
  visitor->attributeEnd(*this, "Left Operand");
  visitor->attributeStart(*this, "Right Operand");
  (*_right_op).accept(visitor);
  visitor->attributeEnd(*this, "Right Operand");
  visitor->nodeEnd(*this);
}

void AR_Load::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Result");
  (*_result).accept(visitor);
  visitor->attributeEnd(*this, "Result");
  visitor->attributeStart(*this, "Pointer");
  (*_ptr).accept(visitor);
  visitor->attributeEnd(*this, "Pointer");
  visitor->nodeEnd(*this);
}

void AR_Return_Value::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  if (_value.getUID() > 0) {
    visitor->attributeStart(*this, "Return Value");
    (*_value).accept(visitor);
    visitor->attributeEnd(*this, "Return Value");
  }
  visitor->nodeEnd(*this);
}

void AR_Unreachable::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->nodeEnd(*this);
}

void AR_Landing_Pad::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Exception Value");
  (*_exception).accept(visitor);
  visitor->attributeEnd(*this, "Exception Value");
  visitor->nodeEnd(*this);
}

void AR_Resume::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Exception Value");
  (*_exception).accept(visitor);
  visitor->attributeEnd(*this, "Exception Value");
  visitor->nodeEnd(*this);
}

void AR_Function_Type::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Return Type");
  (*_return_type).accept(visitor);
  visitor->attributeEnd(*this, "Return Type");
  visitor->attributeStart(*this, "Parameter Types");
  visitor->listStart();
  for (auto p = _param_types.begin(); p != _param_types.end(); p++) {
    (**p).accept(visitor);
    if (p + 1 != _param_types.end()) {
      visitor->listContinued();
    }
  }
  visitor->listEnd();
  visitor->attributeEnd(*this, "Parameter Types");
  visitor->nodeEnd(*this);
}

void AR_Integer_Type::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->nodeEnd(*this);
}

void AR_Structure_Type::accept(std::shared_ptr< Visitor > visitor) {
  if (visitor->checkVisited(getUID()))
    return;

  visitor->markVisited(getUID());
  visitor->nodeStart(*this);
  std::map< z_number, index64_t >::iterator p = _layout.begin();
  for (; p != _layout.end(); p++) {
    std::ostringstream sstream;
    sstream << p->first;
    std::string attr_name = "Offset = " + sstream.str();
    visitor->attributeStart(*this, attr_name);
    AR_Node_Ref< AR_Type > ty_ref(p->second);
    (*ty_ref).accept(visitor);
    visitor->attributeEnd(*this, attr_name);
  }
  visitor->nodeEnd(*this);
}

void AR_Pointer_Type::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Cell Type");
  AR_Node_Ref< AR_Type > ty_ref(_cell_type);
  (*ty_ref).accept(visitor);
  visitor->attributeEnd(*this, "Cell Type");
  visitor->nodeEnd(*this);
}

void AR_Array_Type::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Cell Type");
  AR_Node_Ref< AR_Type > ty_ref(_cell_type);
  (*ty_ref).accept(visitor);
  visitor->attributeEnd(*this, "Cell Type");
  visitor->nodeEnd(*this);
}

void AR_FP_Type::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->nodeEnd(*this);
}

void AR_Void_Type::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->nodeEnd(*this);
}

void AR_Opaque_Type::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->nodeEnd(*this);
}

void AR_Undefined_Constant::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Type");
  (*_type).accept(visitor);
  visitor->attributeEnd(*this, "Type");
  visitor->nodeEnd(*this);
}
void AR_Integer_Constant::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Type");
  (*_type).accept(visitor);
  visitor->attributeEnd(*this, "Type");
  visitor->nodeEnd(*this);
}

void AR_Null_Constant::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Type");
  (*_type).accept(visitor);
  visitor->attributeEnd(*this, "Type");
  visitor->nodeEnd(*this);
}

void AR_FP_Constant::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Type");
  (*_type).accept(visitor);
  visitor->attributeEnd(*this, "Type");
  visitor->nodeEnd(*this);
}

void AR_Range_Constant::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Type");
  (*_type).accept(visitor);
  visitor->attributeEnd(*this, "Type");
  visitor->nodeEnd(*this);
}

void AR_Var_Addr_Constant::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Variable");
  AR_Node_Ref< AR_Variable > var(_variable);
  (*var).accept(visitor);
  visitor->attributeEnd(*this, "Variable");
  visitor->nodeEnd(*this);
}

void AR_Function_Addr_Constant::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->nodeEnd(*this);
}

void AR_Global_Variable::accept(std::shared_ptr< Visitor > visitor) {
  if (visitor->checkVisited(getUID()))
    return;

  visitor->markVisited(getUID());
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Type");
  (*_type).accept(visitor);
  visitor->attributeEnd(*this, "Type");
  /*
  TODO: Write a generic visitor
  if (_initializer.getUID() > 0) {
    visitor->attributeStart(*this, "Initializer");
    (*_initializer).accept(visitor);
    visitor->attributeEnd(*this, "Initializer");
  }
  */
  visitor->nodeEnd(*this);
}

void AR_Local_Variable::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Type");
  (*_type).accept(visitor);
  visitor->attributeEnd(*this, "Type");
  visitor->nodeEnd(*this);
}

void AR_Cst_Operand::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Constant");
  (*_constant).accept(visitor);
  visitor->attributeEnd(*this, "Constant");
  visitor->nodeEnd(*this);
}

void AR_Var_Operand::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Internal Variable");
  (*_internal_variable).accept(visitor);
  visitor->attributeEnd(*this, "Internal Variable");
  visitor->nodeEnd(*this);
}

void AR_Internal_Variable::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Type");
  (*_type).accept(visitor);
  visitor->attributeEnd(*this, "Type");
  visitor->nodeEnd(*this);
}

void AR_Arith_Op::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Result");
  (*_result).accept(visitor);
  visitor->attributeEnd(*this, "Result");
  visitor->attributeStart(*this, "Left Operand");
  (*_left_op).accept(visitor);
  visitor->attributeEnd(*this, "Left Operand");
  visitor->attributeStart(*this, "Right Operand");
  (*_right_op).accept(visitor);
  visitor->attributeEnd(*this, "Right Operand");
  visitor->nodeEnd(*this);
}

void AR_Conv_Op::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Operand");
  (*_operand).accept(visitor);
  visitor->attributeEnd(*this, "Operand");
  visitor->attributeStart(*this, "Result");
  (*_result).accept(visitor);
  visitor->attributeEnd(*this, "Result");
  visitor->nodeEnd(*this);
}

void AR_Bitwise_Op::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Result");
  (*_result).accept(visitor);
  visitor->attributeEnd(*this, "Result");
  visitor->attributeStart(*this, "Left Operand");
  (*_left_op).accept(visitor);
  visitor->attributeEnd(*this, "Left Operand");
  visitor->attributeStart(*this, "Right Operand");
  (*_right_op).accept(visitor);
  visitor->attributeEnd(*this, "Right Operand");
  visitor->nodeEnd(*this);
}

void AR_FP_Op::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Result");
  (*_result).accept(visitor);
  visitor->attributeEnd(*this, "Result");
  visitor->attributeStart(*this, "Left Operand");
  (*_left_op).accept(visitor);
  visitor->attributeEnd(*this, "Left Operand");
  visitor->attributeStart(*this, "Right Operand");
  (*_right_op).accept(visitor);
  visitor->attributeEnd(*this, "Right Operand");
  visitor->nodeEnd(*this);
}

void AR_Allocate::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Result");
  (*_local_var_ref).accept(visitor);
  visitor->attributeEnd(*this, "Result");
  visitor->attributeStart(*this, "Array Size");
  (*_array_size).accept(visitor);
  visitor->attributeEnd(*this, "Array Size");
  visitor->nodeEnd(*this);
}

void AR_Store::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Value");
  (*_value).accept(visitor);
  visitor->attributeEnd(*this, "Value");
  visitor->attributeStart(*this, "Pointer");
  (*_ptr).accept(visitor);
  visitor->attributeEnd(*this, "Pointer");
  visitor->nodeEnd(*this);
}

void AR_Extract_Element::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Result");
  (*_result).accept(visitor);
  visitor->attributeEnd(*this, "Result");
  visitor->attributeStart(*this, "Value");
  (*_value).accept(visitor);
  visitor->attributeEnd(*this, "Value");
  visitor->attributeStart(*this, "Offset");
  (*_offset).accept(visitor);
  visitor->attributeEnd(*this, "Offset");
  visitor->nodeEnd(*this);
}

void AR_Insert_Element::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Result");
  (*_result).accept(visitor);
  visitor->attributeEnd(*this, "Result");
  visitor->attributeStart(*this, "Value");
  (*_value).accept(visitor);
  visitor->attributeEnd(*this, "Value");
  visitor->attributeStart(*this, "Element");
  (*_element).accept(visitor);
  visitor->attributeEnd(*this, "Element");
  visitor->attributeStart(*this, "Offset");
  (*_offset).accept(visitor);
  visitor->attributeEnd(*this, "Offset");
  visitor->nodeEnd(*this);
}

void AR_Pointer_Shift::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Result");
  (*_result).accept(visitor);
  visitor->attributeEnd(*this, "Result");
  visitor->attributeStart(*this, "Offset");
  (*_offset).accept(visitor);
  visitor->attributeEnd(*this, "Offset");
  visitor->attributeStart(*this, "Pointer");
  (*_ptr).accept(visitor);
  visitor->attributeEnd(*this, "Pointer");
  visitor->nodeEnd(*this);
}

void AR_Call::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  if (_return_value.getUID() > 0) {
    visitor->attributeStart(*this, "Return Value");
    (*_return_value).accept(visitor);
    visitor->attributeEnd(*this, "Return Value");
  }
  visitor->attributeStart(*this, "Called Function");
  (*_called_function).accept(visitor);
  visitor->attributeEnd(*this, "Called Function");

  visitor->attributeStart(*this, "Arguments");
  visitor->listStart();
  for (std::vector< AR_Node_Ref< AR_Operand > >::iterator p = _args.begin();
       p != _args.end();
       p++) {
    (**p).accept(visitor);
    if (p + 1 != _args.end()) {
      visitor->listContinued();
    }
  }
  visitor->attributeEnd(*this, "Arguments");
  visitor->listEnd();
  visitor->nodeEnd(*this);
}

void AR_Invoke::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  /*
  TODO: Write a generic visitor
  (*_call).accept(visitor);
  visitor->attributeStart(*this, "Normal");
  AR_Node_Ref< AR_Basic_Block > normal_bb(_normal);
  (*normal_bb).accept(visitor);
  visitor->attributeEnd(*this, "Normal");
  visitor->attributeStart(*this, "Exception");
  AR_Node_Ref< AR_Basic_Block > exception_bb(_exception);
  (*exception_bb).accept(visitor);
  visitor->attributeEnd(*this, "Exception");
  */
  visitor->nodeEnd(*this);
}

void AR_NOP::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->nodeEnd(*this);
}

void AR_MemSet::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Operand");
  (*_ptr).accept(visitor);
  visitor->attributeEnd(*this, "Operand");
  visitor->attributeStart(*this, "Length");
  (*_len).accept(visitor);
  visitor->attributeEnd(*this, "Length");
  visitor->attributeStart(*this, "Value");
  (*_value).accept(visitor);
  visitor->attributeEnd(*this, "Value");
  visitor->nodeEnd(*this);
}

void AR_MemCpy::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Target");
  (*_tgt_pointer).accept(visitor);
  visitor->attributeEnd(*this, "Target");
  visitor->attributeStart(*this, "Source");
  (*_src_pointer).accept(visitor);
  visitor->attributeEnd(*this, "Source");
  visitor->attributeStart(*this, "Length");
  (*_len).accept(visitor);
  visitor->attributeEnd(*this, "Length");
  visitor->nodeEnd(*this);
}

void AR_MemMove::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Target");
  (*_tgt_pointer).accept(visitor);
  visitor->attributeEnd(*this, "Target");
  visitor->attributeStart(*this, "Source");
  (*_src_pointer).accept(visitor);
  visitor->attributeEnd(*this, "Source");
  visitor->attributeStart(*this, "Length");
  (*_len).accept(visitor);
  visitor->attributeEnd(*this, "Length");
  visitor->nodeEnd(*this);
}

void AR_VA_Start::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "va_list_ptr");
  (*_va_list_ptr).accept(visitor);
  visitor->attributeEnd(*this, "va_list_ptr");
  visitor->nodeEnd(*this);
}

void AR_VA_End::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "va_list_ptr");
  (*_va_list_ptr).accept(visitor);
  visitor->attributeEnd(*this, "va_list_ptr");
  visitor->nodeEnd(*this);
}

void AR_VA_Copy::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "dest_va_list_ptr");
  (*_dest_va_list_ptr).accept(visitor);
  visitor->attributeEnd(*this, "dest_va_list_ptr");
  visitor->attributeStart(*this, "src_va_list_ptr");
  (*_src_va_list_ptr).accept(visitor);
  visitor->attributeEnd(*this, "src_va_list_ptr");
  visitor->nodeEnd(*this);
}

void AR_VA_Arg::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "result");
  (*_result).accept(visitor);
  visitor->attributeEnd(*this, "result");
  visitor->attributeStart(*this, "va_list_ptr");
  (*_va_list_ptr).accept(visitor);
  visitor->attributeEnd(*this, "va_list_ptr");
  visitor->nodeEnd(*this);
}

void AR_Abstract_Memory::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "ptr");
  (*_ptr).accept(visitor);
  visitor->attributeEnd(*this, "ptr");
  visitor->attributeStart(*this, "len");
  (*_len).accept(visitor);
  visitor->attributeEnd(*this, "len");
  visitor->nodeEnd(*this);
}

void AR_Abstract_Variable::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "var");
  (*_variable).accept(visitor);
  visitor->attributeEnd(*this, "var");
  visitor->nodeEnd(*this);
}

void AR_Basic_Block::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Body");
  visitor->listStart();
  for (std::vector< AR_Node_Ref< AR_Statement > >::iterator p = _body.begin();
       p != _body.end();
       p++) {
    (**p).accept(visitor);
    if (p + 1 != _body.end())
      visitor->listContinued();
  }
  visitor->listEnd();
  visitor->attributeEnd(*this, "Body");
  visitor->nodeEnd(*this);
}

void AR_Code::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Blocks");
  visitor->listStart();
  for (std::vector< AR_Node_Ref< AR_Basic_Block > >::iterator p =
           _blocks.begin();
       p != _blocks.end();
       p++) {
    (**p).accept(visitor);
    if (p + 1 != _blocks.end())
      visitor->listContinued();
  }
  visitor->listEnd();
  visitor->attributeEnd(*this, "Blocks");
  visitor->nodeEnd(*this);
}

void AR_Function::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Function Body");
  (*_function_body).accept(visitor);
  visitor->attributeEnd(*this, "Function Body");
  visitor->attributeStart(*this, "Function Type");
  (*_function_type).accept(visitor);
  visitor->attributeEnd(*this, "Function Type");
  visitor->nodeEnd(*this);
}

void AR_Bundle::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->attributeStart(*this, "Global Variables");
  visitor->listStart();
  for (std::vector< AR_Node_Ref< AR_Global_Variable > >::iterator p =
           _globals.begin();
       p != _globals.end();
       p++) {
    (**p).accept(visitor);
    if (p + 1 != _globals.end()) {
      visitor->listContinued();
    }
  }
  visitor->listEnd();
  visitor->attributeEnd(*this, "Global Variables");

  visitor->attributeStart(*this, "Functions");
  visitor->listStart();
  for (std::vector< AR_Node_Ref< AR_Function > >::iterator p =
           _functions.begin();
       p != _functions.end();
       p++) {
    (**p).accept(visitor);
    if (p + 1 != _functions.end()) {
      visitor->listContinued();
    }
  }
  visitor->listEnd();
  visitor->attributeEnd(*this, "Functions");
  visitor->nodeEnd(*this);
}

void AR_Source_Location::accept(std::shared_ptr< Visitor > visitor) {
  visitor->nodeStart(*this);
  visitor->nodeEnd(*this);
}

void ARModel::accept(std::shared_ptr< Visitor > visitor) {
  (*_bundle).accept(visitor);
}
