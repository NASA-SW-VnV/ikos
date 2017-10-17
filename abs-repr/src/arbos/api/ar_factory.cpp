/*******************************************************************************
 *
 * Factory methods to create ARBOS objects
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

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include <arbos/semantics/ar.hpp>

using namespace arbos;

const std::string ARFactory::INTEGER_TYPE = "int";
const std::string ARFactory::FLOAT_TYPE = "float";
const std::string ARFactory::FUNCTION_TYPE = "function";
const std::string ARFactory::POINTER_TYPE = "ptr";
const std::string ARFactory::ARRAY_TYPE = "array";
const std::string ARFactory::STRUCT_TYPE = "struct";
const std::string ARFactory::VOID_TYPE = "void";
const std::string ARFactory::OPAQUE_TYPE = "opaque";

AR_Node_Ref< AR_Type > ARFactory::createType(index64_t id) {
  if (id > 0) {
    index64_t uid = ARModel::Instance()->getARfromInputRef(id);
    if (uid > 0) {
      AR_Node_Ref< AR_Type > type(uid);
      if (type.getUID() > 0) {
        return type;
      } else {
        std::cerr << "Node #" << uid << " input_ref = " << id
                  << " has been garbage-collected." << std::endl;
      }
    }
  }

  s_expression e = ARModel::Instance()->getSEXPRfromInputRef(id)[1];
  index64_t currentUID = UIDGenerator::getCurrentUID();
  ARModel::Instance()->registerARtoInputRef(currentUID, id);
  std::string kind = (static_cast< string_atom& >(*e)).data();
  AR_Node_Ref< AR_Type > type = AR_Null_Ref;

  if (kind == FUNCTION_TYPE) {
    type = AR_Function_Type::create(e);
  } else if (kind == INTEGER_TYPE) {
    type = AR_Integer_Type::create(e);
  } else if (kind == FLOAT_TYPE) {
    type = AR_FP_Type::create(e);
  } else if (kind == POINTER_TYPE) {
    type = AR_Pointer_Type::create(e);
  } else if (kind == ARRAY_TYPE) {
    type = AR_Array_Type::create(e);
  } else if (kind == STRUCT_TYPE) {
    type = AR_Structure_Type::create(e);
  } else if (kind == VOID_TYPE) {
    type = AR_Void_Type::create(e);
  } else if (kind == OPAQUE_TYPE) {
    // AR format: ($type $opaque), where range of
    // the opaque structure is unknown, thus size = 0.
    type = AR_Opaque_Type::create(0);
  }

  if (type == AR_Null_Ref) {
    std::ostringstream ss;
    ss << "Cannot create type with \"" << e << "\"" << std::endl;
    throw error(ss.str());
  }

  assert(id == 0 || currentUID == type.getUID());
  return type;
}

// Conversion operation types
const std::string ARFactory::TRUNC_OP = "trunc";
const std::string ARFactory::Z_EXT_OP = "zext";
const std::string ARFactory::S_EXT_OP = "sext";
const std::string ARFactory::FP_TRUNC_OP = "fptrunc";
const std::string ARFactory::FP_EXT_OP = "fpext";
const std::string ARFactory::FP_TO_UI_OP = "fptoui";
const std::string ARFactory::FP_TO_SI_OP = "fptosi";
const std::string ARFactory::UI_TO_FP_OP = "uitofp";
const std::string ARFactory::SI_TO_FP_OP = "sitofp";
const std::string ARFactory::PTR_TO_INT_OP = "ptrtoint";
const std::string ARFactory::INT_TO_PTR_OP = "inttoptr";
const std::string ARFactory::BITCAST_OP = "bitcast";

// Bitwise operations
const std::string ARFactory::BITWISE_SHL = "shl";
const std::string ARFactory::BITWISE_LSHR = "lshr";
const std::string ARFactory::BITWISE_ASHR = "ashr";
const std::string ARFactory::BITWISE_AND = "and";
const std::string ARFactory::BITWISE_OR = "or";
const std::string ARFactory::BITWISE_XOR = "xor";

// FP operations
const std::string ARFactory::FADD = "fadd";
const std::string ARFactory::FSUB = "fsub";
const std::string ARFactory::FMUL = "fmul";
const std::string ARFactory::FDIV = "fdiv";
const std::string ARFactory::FREM = "frem";

// Arithmetic operation types
const std::string ARFactory::ADD_OP = "add";
const std::string ARFactory::SUB_OP = "sub";
const std::string ARFactory::MUL_OP = "mul";
const std::string ARFactory::UDIV_OP = "udiv";
const std::string ARFactory::SDIV_OP = "sdiv";
const std::string ARFactory::UREM_OP = "urem";
const std::string ARFactory::SREM_OP = "srem";

// Operand types
const std::string ARFactory::CONSTANT_TYPE = "cst";
const std::string ARFactory::VAR_TYPE = "var";

// Register operators
const std::string ARFactory::ASSIGN = "assign";

// Register operators for aggregate types
const std::string ARFactory::EXTRACT_ELEMENT = "extractelem";
const std::string ARFactory::INSERT_ELEMENT = "insertelem";

// Assertion operators
const std::string ARFactory::ASSERT = "assert";

// Stack operations
const std::string ARFactory::ALLOCATE_OP = "allocate";

// Pointer operations
const std::string ARFactory::POINTER_SHIFT_OP = "ptrshift";

// Abstract operations
const std::string ARFactory::ABSTRACT_VARIABLE = "abstractvariable";
const std::string ARFactory::ABSTRACT_MEMORY = "abstractmemory";

// Memory operations
const std::string ARFactory::LOAD_OP = "load";
const std::string ARFactory::STORE_OP = "store";
const std::string ARFactory::MEMSET = "memset";
const std::string ARFactory::MEMCOPY = "memcpy";
const std::string ARFactory::MEMMOVE = "memmove";

// Return statement
const std::string ARFactory::RET_OP = "ret";

// Function call
const std::string ARFactory::CALL = "call";
const std::string ARFactory::INVOKE = "invoke";

// Variable arguments
const std::string ARFactory::VA_START = "va_start";
const std::string ARFactory::VA_END = "va_end";
const std::string ARFactory::VA_COPY = "va_copy";

// Exception handling
const std::string ARFactory::LANDINGPAD = "landingpad";
const std::string ARFactory::RESUME = "resume";

// Other statements
const std::string ARFactory::UNREACHABLE = "unreachable";

std::vector< AR_Node_Ref< AR_Statement > > ARFactory::createStatements(
    index64_t parent_bblock, s_expression e) {
  std::vector< AR_Node_Ref< AR_Statement > > stmts;

  if (s_pattern(ASSIGN) ^ e) {
    stmts.push_back(AR_Assignment::create(parent_bblock, e));
  } else if (s_pattern(ASSERT) ^ e) {
    /**
     * Example s-expr: ($assert ($kind ($i))
     ($cmp ($cond ($true))
     ($pred ($eq))
     ($ops ($var ($name ($eh_select)) ($ty (!24))) ($var ($name ($eh_typeid))
     ($ty (!24)))))
     ($debug ($srcloc ($line (#48)) ($file (!27)))))
     */
    s_expression_ref k, cmp, srcloc;
    if (s_pattern(ASSERT,
                  s_pattern("kind", k),
                  cmp,
                  s_pattern("debug", srcloc)) ^
        e) {
      AR_Node_Ref< AR_Source_Location > source_location =
          ARFactory::createSourceLocation(*srcloc);
      std::string kind = (static_cast< string_atom& >(**k)).data();
      if (kind == "i") {
        AR_Node_Ref< AR_Integer_Comparison > icmp =
            AR_Integer_Comparison::create(parent_bblock, *cmp);
        icmp->setSourceLocation(source_location);
        stmts.push_back(icmp);
      } else if (kind == "fp") {
        AR_Node_Ref< AR_FP_Comparison > fcmp =
            AR_FP_Comparison::create(parent_bblock, *cmp);
        fcmp->setSourceLocation(source_location);
        stmts.push_back(fcmp);
      } else {
        throw error("Invalid data type \"" + kind + "\" for assertion");
      }
    } else {
      throw parse_error("ARFactory::createStatements() for assert", e);
    }
  } else if (s_pattern(ADD_OP) ^ e) {
    stmts.push_back(AR_Arith_Op::create(parent_bblock, e, add));
  } else if (s_pattern(SUB_OP) ^ e) {
    stmts.push_back(AR_Arith_Op::create(parent_bblock, e, sub));
  } else if (s_pattern(MUL_OP) ^ e) {
    stmts.push_back(AR_Arith_Op::create(parent_bblock, e, mul));
  } else if (s_pattern(UDIV_OP) ^ e) {
    stmts.push_back(AR_Arith_Op::create(parent_bblock, e, udiv));
  } else if (s_pattern(SDIV_OP) ^ e) {
    stmts.push_back(AR_Arith_Op::create(parent_bblock, e, sdiv));
  } else if (s_pattern(UREM_OP) ^ e) {
    stmts.push_back(AR_Arith_Op::create(parent_bblock, e, urem));
  } else if (s_pattern(SREM_OP) ^ e) {
    stmts.push_back(AR_Arith_Op::create(parent_bblock, e, srem));
  } else if (s_pattern(FADD) ^ e) {
    stmts.push_back(AR_FP_Op::create(parent_bblock, e, fadd));
  } else if (s_pattern(FSUB) ^ e) {
    stmts.push_back(AR_FP_Op::create(parent_bblock, e, fsub));
  } else if (s_pattern(FMUL) ^ e) {
    stmts.push_back(AR_FP_Op::create(parent_bblock, e, fmul));
  } else if (s_pattern(FDIV) ^ e) {
    stmts.push_back(AR_FP_Op::create(parent_bblock, e, fdiv));
  } else if (s_pattern(FREM) ^ e) {
    stmts.push_back(AR_FP_Op::create(parent_bblock, e, frem));
  } else if (s_pattern(TRUNC_OP) ^ e) {
    stmts.push_back(AR_Conv_Op::create(parent_bblock, e, trunc));
  } else if (s_pattern(Z_EXT_OP) ^ e) {
    stmts.push_back(AR_Conv_Op::create(parent_bblock, e, zext));
  } else if (s_pattern(S_EXT_OP) ^ e) {
    stmts.push_back(AR_Conv_Op::create(parent_bblock, e, sext));
  } else if (s_pattern(FP_TRUNC_OP) ^ e) {
    stmts.push_back(AR_Conv_Op::create(parent_bblock, e, fptrunc));
  } else if (s_pattern(FP_EXT_OP) ^ e) {
    stmts.push_back(AR_Conv_Op::create(parent_bblock, e, fpext));
  } else if (s_pattern(FP_TO_UI_OP) ^ e) {
    stmts.push_back(AR_Conv_Op::create(parent_bblock, e, fptoui));
  } else if (s_pattern(FP_TO_SI_OP) ^ e) {
    stmts.push_back(AR_Conv_Op::create(parent_bblock, e, fptosi));
  } else if (s_pattern(UI_TO_FP_OP) ^ e) {
    stmts.push_back(AR_Conv_Op::create(parent_bblock, e, uitofp));
  } else if (s_pattern(SI_TO_FP_OP) ^ e) {
    stmts.push_back(AR_Conv_Op::create(parent_bblock, e, sitofp));
  } else if (s_pattern(PTR_TO_INT_OP) ^ e) {
    stmts.push_back(AR_Conv_Op::create(parent_bblock, e, ptrtoint));
  } else if (s_pattern(INT_TO_PTR_OP) ^ e) {
    stmts.push_back(AR_Conv_Op::create(parent_bblock, e, inttoptr));
  } else if (s_pattern(BITCAST_OP) ^ e) {
    stmts.push_back(AR_Conv_Op::create(parent_bblock, e, bitcast));
  } else if (s_pattern(BITWISE_SHL) ^ e) {
    stmts.push_back(AR_Bitwise_Op::create(parent_bblock, e, _shl));
  } else if (s_pattern(BITWISE_LSHR) ^ e) {
    stmts.push_back(AR_Bitwise_Op::create(parent_bblock, e, _lshr));
  } else if (s_pattern(BITWISE_ASHR) ^ e) {
    stmts.push_back(AR_Bitwise_Op::create(parent_bblock, e, _ashr));
  } else if (s_pattern(BITWISE_AND) ^ e) {
    stmts.push_back(AR_Bitwise_Op::create(parent_bblock, e, _and));
  } else if (s_pattern(BITWISE_OR) ^ e) {
    stmts.push_back(AR_Bitwise_Op::create(parent_bblock, e, _or));
  } else if (s_pattern(BITWISE_XOR) ^ e) {
    stmts.push_back(AR_Bitwise_Op::create(parent_bblock, e, _xor));
  } else if (s_pattern(LOAD_OP) ^ e) {
    stmts.push_back(AR_Load::create(parent_bblock, e));
  } else if (s_pattern(STORE_OP) ^ e) {
    stmts.push_back(AR_Store::create(parent_bblock, e));
  } else if (s_pattern(ALLOCATE_OP) ^ e) {
    stmts.push_back(AR_Allocate::create(parent_bblock, e));
  } else if (s_pattern(MEMSET) ^ e) {
    stmts.push_back(AR_MemSet::create(parent_bblock, e));
  } else if (s_pattern(MEMCOPY) ^ e) {
    stmts.push_back(AR_MemCpy::create(parent_bblock, e));
  } else if (s_pattern(MEMMOVE) ^ e) {
    stmts.push_back(AR_MemMove::create(parent_bblock, e));
  } else if (s_pattern(VA_START) ^ e) {
    stmts.push_back(AR_VA_Start::create(parent_bblock, e));
  } else if (s_pattern(VA_END) ^ e) {
    stmts.push_back(AR_VA_End::create(parent_bblock, e));
  } else if (s_pattern(VA_COPY) ^ e) {
    stmts.push_back(AR_VA_Copy::create(parent_bblock, e));
  } else if (s_pattern(RET_OP) ^ e) {
    stmts.push_back(AR_Return_Value::create(parent_bblock, e));
  } else if (s_pattern(CALL) ^ e) {
    stmts.push_back(AR_Call::create(parent_bblock, e));
  } else if (s_pattern(INVOKE) ^ e) {
    stmts.push_back(AR_Invoke::create(parent_bblock, e));
  } else if (s_pattern(ABSTRACT_VARIABLE) ^ e) {
    stmts.push_back(AR_Abstract_Variable::create(parent_bblock, e));
  } else if (s_pattern(ABSTRACT_MEMORY) ^ e) {
    stmts.push_back(AR_Abstract_Memory::create(parent_bblock, e));
  } else if (s_pattern(LANDINGPAD) ^ e) {
    stmts.push_back(AR_Landing_Pad::create(parent_bblock, e));
  } else if (s_pattern(RESUME) ^ e) {
    stmts.push_back(AR_Resume::create(parent_bblock, e));
  } else if (s_pattern(POINTER_SHIFT_OP) ^ e) {
    stmts.push_back(AR_Pointer_Shift::create(parent_bblock, e));
  } else if (s_pattern(EXTRACT_ELEMENT) ^ e) {
    stmts.push_back(AR_Extract_Element::create(parent_bblock, e));
  } else if (s_pattern(INSERT_ELEMENT) ^ e) {
    stmts.push_back(AR_Insert_Element::create(parent_bblock, e));
  } else if (s_pattern(UNREACHABLE) ^ e) {
    stmts.push_back(AR_Unreachable::create(parent_bblock, e));
  } else {
    std::ostringstream ss;
    ss << "Cannot create statement for " << e;
    throw error(ss.str());
  }
  return stmts;
}

AR_Node_Ref< AR_Operand > ARFactory::createOperand(s_expression e) {
  /**
   * Example s-expr: ($cst ($constantint ($val (#0)) ($ty (!6))))
   *         ($var ($name ($i.0)) ($ty (!9)))
   */
  if (s_pattern(CONSTANT_TYPE) ^ e) {
    return AR_Cst_Operand::create(e);
  } else if (s_pattern(VAR_TYPE) ^ e) {
    return AR_Var_Operand::create(e);
  } else {
    throw parse_error("ARFactory::createOperand()", e);
  }
}

const std::string ARFactory::CST_INTEGER = "constantint";
const std::string ARFactory::CST_FLOAT = "constantfp";
const std::string ARFactory::CST_RANGE = "range";
const std::string ARFactory::CST_POINTER_NULL = "constantpointernull";
const std::string ARFactory::GLOBAL_VAR_REF = "globalvariableref";
const std::string ARFactory::LOCAL_VAR_REF = "localvariableref";
const std::string ARFactory::CST_FUNCTION_PTR = "constantfunctionptr";
const std::string ARFactory::CST_UNDEFINED = "undefined";

AR_Node_Ref< AR_Constant > ARFactory::createConstant(s_expression e) {
  if (s_pattern(CST_INTEGER, s_pattern(), s_pattern()) ^ e) {
    return AR_Integer_Constant::create(e);
  } else if (s_pattern(CST_FLOAT, s_pattern(), s_pattern()) ^ e) {
    return AR_FP_Constant::create(e);
  } else if (s_pattern(CST_UNDEFINED, s_pattern()) ^ e) {
    return AR_Undefined_Constant::create(e);
  } else if (s_pattern(GLOBAL_VAR_REF, s_pattern(), s_pattern(), s_pattern()) ^
             e) {
    return AR_Var_Addr_Constant::create(e);
  } else if (s_pattern(LOCAL_VAR_REF) ^ e) {
    return AR_Var_Addr_Constant::create(e);
  } else if (s_pattern(CST_FUNCTION_PTR) ^ e) {
    return AR_Function_Addr_Constant::create(e);
  } else if (s_pattern(CST_POINTER_NULL) ^ e) {
    return AR_Null_Constant::create(e);
  } else if (s_pattern(CST_RANGE) ^ e) {
    return AR_Range_Constant::create(e);
  } else {
    std::ostringstream ss;
    ss << "Cannot create constant with \"" << e << "\"" << std::endl;
    throw error(ss.str());
  }
}

AR_Node_Ref< AR_Global_Variable > ARFactory::createGlobalVariable(
    index64_t id) {
  /**
   * Example s-expr: ($gv ($name ($.str)) ($ty (!2)) ($code ($basicblocks)
   * ($trans)) ($debug ($srcloc)))
   */
  if (id > 0) {
    index64_t uid = ARModel::Instance()->getARfromInputRef(id);
    if (uid > 0) {
      AR_Node_Ref< AR_Global_Variable > gv(uid);
      if (gv.getUID() > 0) {
        return gv;
      } else {
        std::cerr << "Node #" << uid << " input_ref = " << id
                  << " has been garbage-collected." << std::endl;
      }
    }
  }

  index64_t currentUID = UIDGenerator::getCurrentUID();
  ARModel::Instance()->registerARtoInputRef(currentUID, id);

  s_expression e = ARModel::Instance()->getSEXPRfromInputRef(id);
  AR_Node_Ref< AR_Global_Variable > gv = AR_Global_Variable::create(e);
  (*ARModel::Instance()->getBundle()).addGlobalVariable(gv);

  if (gv.getUID() > 0 && id > 0) {
    assert(currentUID == gv.getUID());
  }

  return gv;
}

AR_Node_Ref< AR_Source_Location > ARFactory::createSourceLocation(
    s_expression e) {
  /**
   * Example s-expr: ($srcloc ($line (#1)) ($file (!6)))
   */
  return AR_Source_Location::create(e);
}

AR_Node_Ref< AR_Code > ARFactory::createCode(index64_t parent_function,
                                             s_expression e) {
  return AR_Code::create(parent_function, e);
}

AR_Node_Ref< AR_Function > ARFactory::createFunction(s_expression e) {
  return AR_Function::create(e);
}

AR_Node_Ref< AR_Basic_Block > ARFactory::createBasicBlock(index64_t parent_code,
                                                          s_expression e) {
  return AR_Basic_Block::create(parent_code, e);
}

AR_Node_Ref< AR_Internal_Variable > ARFactory::createInternalVariable(
    s_expression e) {
  // AR_Internal_Variables must be unique by the IDs given from the LLVM
  // ARBOSPass.
  AR_Node_Ref< AR_Code > scope = ARModel::Instance()->getCurrentBuildingScope();
  AR_Node_Ref< AR_Internal_Variable > internal_var =
      AR_Internal_Variable::create(e);
  // Use existing internal variable with the same name given by the LLVM
  // ARBOSPass.
  AR_Node_Ref< AR_Internal_Variable > existing_internal_var =
      (*scope).getInternalVariable((*internal_var).getName());

  if (existing_internal_var.getUID() > 0)
    return existing_internal_var;
  else {
    (*ARModel::Instance()->getCurrentBuildingScope())
        .addInternalVariable(internal_var);
    return internal_var;
  }
}

ARModel* ARModel::_instance = new ARModel();
