/*******************************************************************************
 *
 * Factory methods to create ARBOS objects
 *
 * Authors: Nija Shi
 *          Arnaud J. Venet
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2016 United States Government as represented by the
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

const std::string ARFactory::BOOL_TYPE = "bool";
const std::string ARFactory::INTEGER_TYPE = "int";
const std::string ARFactory::FLOAT_TYPE = "float";
const std::string ARFactory::FUNCTION_TYPE = "function";
const std::string ARFactory::POINTER_TYPE = "ptr";
const std::string ARFactory::ARRAY_TYPE = "array";
const std::string ARFactory::STRUCT_TYPE = "struct";
const std::string ARFactory::VOID_TYPE = "void";
const std::string ARFactory::OPAQUE_TYPE = "opaque";
// XAR Types
const std::string ARFactory::LABEL_TYPE = "label";

AR_Node_Ref< AR_Void_Type > ARFactory::VOID_TYPE_AR_NODE;
AR_Node_Ref< AR_Undefined_Constant > ARFactory::VOID_UNDEFINED_CONSTANT_AR_NODE;

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
  AR_Node_Ref< AR_Type > type;

  if (kind == FUNCTION_TYPE) {
    type.setUID(AR_Function_Type::create(e)->getUID());
  } else if (kind == INTEGER_TYPE) {
    std::shared_ptr< AR_Integer_Type > int_type(AR_Integer_Type::create(e));
    type.setUID(int_type->getUID());
  } else if (kind == FLOAT_TYPE) {
    type.setUID(AR_FP_Type::create(e)->getUID());
  } else if (kind == POINTER_TYPE) {
    type.setUID(AR_Pointer_Type::create(e)->getUID());
  } else if (kind == ARRAY_TYPE) {
    type.setUID(AR_Array_Type::create(e)->getUID());
  } else if (kind == STRUCT_TYPE) {
    type.setUID(AR_Structure_Type::create(e)->getUID());
  } else if (kind == VOID_TYPE) {
    type.setUID(AR_Void_Type::create(e)->getUID());
  } else if (kind == OPAQUE_TYPE) {
    // AR format: ($type $opaque), where range of
    // the opaque structure is unknown, thus size = 0.
    type.setUID(AR_Opaque_Type::create(0)->getUID());
  }

  if (type.getUID() == 0) {
    std::ostringstream ss;
    ss << "Cannot create type with \"" << e << "\"" << std::endl;
    throw error(ss.str());
  }

  if (type.getUID() > 0 && id > 0) {
    assert(currentUID == type.getUID());
  }

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
const std::string ARFactory::CONSTANT_TYPE = "$constant";
const std::string ARFactory::VAR_TYPE = "var";
const std::string ARFactory::IDX_TYPE = "$idx";
const std::string ARFactory::NAMEDVAR_TYPE = "$namedvar";
const std::string ARFactory::GLOBALVAR_TYPE = "$global_var";

// Register operators
const std::string ARFactory::ASSIGN = "assign";

// Register operators for aggregate types
const std::string ARFactory::INSERT_VALUE = "$insertvalue";
const std::string ARFactory::EXTRACT_VALUE = "$extractvalue";

// Assertion operators
const std::string ARFactory::ASSERT = "assert";

// Pointer operation types
const std::string ARFactory::ALLOCA_OP = "$alloc";

// Memory operations
const std::string ARFactory::LOAD_OP = "load";
const std::string ARFactory::STORE_OP = "store";

// Return statement
const std::string ARFactory::RET_OP = "ret";

// XAR statements
const std::string ARFactory::XAR_BR_OP = "$br";
const std::string ARFactory::XAR_GETELEMETRYPOINTER_OP = "getelementptr";
const std::string ARFactory::XAR_STORE_OP = "$store";
const std::string ARFactory::XAR_LOAD_OP = "$load";
const std::string ARFactory::XAR_PHI_OP = "$phi";
const std::string ARFactory::XAR_ICMP_OP = "$icmp";
const std::string ARFactory::XAR_FCMP_OP = "$fcmp";

// Function call
const std::string ARFactory::CALL = "call";

// Exception handling
const std::string ARFactory::LANDINGPAD = "landingpad";
const std::string ARFactory::RESUME = "resume";

// Other statements
const std::string ARFactory::UNREACHABLE = "$unreachable";
const std::string ARFactory::RANDOMIZE = "$randomize";

std::vector< AR_Node_Ref< AR_Statement > > ARFactory::createStatements(
    index64_t parent_bblock, s_expression e) {
  std::vector< AR_Node_Ref< AR_Statement > > stmts;
  if (s_pattern(ASSIGN) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Assignment::create(parent_bblock, e)->getUID()));
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
    if (s_pattern("assert",
                  s_pattern("kind", k),
                  cmp,
                  s_pattern("debug", srcloc)) ^
        e) {
      AR_Node_Ref< AR_Source_Location > source_location =
          ARFactory::createSourceLocation(*srcloc);
      AR_Node_Ref< AR_Statement > stmt;
      std::string kind = (static_cast< string_atom& >(**k)).data();
      if (kind == "i") {
        std::shared_ptr< AR_Integer_Comparison > icmp(
            AR_Integer_Comparison::create(parent_bblock, *cmp));
        (*icmp).setSourceLocation(source_location);
        stmt.setUID(icmp->getUID());
      } else if (kind == "fp") {
        std::shared_ptr< AR_FP_Comparison > fcmp(
            AR_FP_Comparison::create(parent_bblock, *cmp));
        (*fcmp).setSourceLocation(source_location);
        stmt.setUID(fcmp->getUID());
      } else
        throw error("Invalid data type \"" + kind + "\" for assertion");
      stmts.push_back(stmt);
    } else
      throw parse_error("ARFactory::createStatements() for assert", e);
  } else if (s_pattern(ADD_OP) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Arith_Op::create(parent_bblock, e, add)->getUID()));
  } else if (s_pattern(SUB_OP) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Arith_Op::create(parent_bblock, e, sub)->getUID()));
  } else if (s_pattern(MUL_OP) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Arith_Op::create(parent_bblock, e, mul)->getUID()));
  } else if (s_pattern(UDIV_OP) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Arith_Op::create(parent_bblock, e, udiv)->getUID()));
  } else if (s_pattern(SDIV_OP) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Arith_Op::create(parent_bblock, e, sdiv)->getUID()));
  } else if (s_pattern(UREM_OP) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Arith_Op::create(parent_bblock, e, urem)->getUID()));
  } else if (s_pattern(SREM_OP) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Arith_Op::create(parent_bblock, e, srem)->getUID()));
  } else if (s_pattern(FADD) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_FP_Op::create(parent_bblock, e, fadd)->getUID()));
  } else if (s_pattern(FSUB) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_FP_Op::create(parent_bblock, e, fsub)->getUID()));
  } else if (s_pattern(FMUL) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_FP_Op::create(parent_bblock, e, fmul)->getUID()));
  } else if (s_pattern(FDIV) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_FP_Op::create(parent_bblock, e, fdiv)->getUID()));
  } else if (s_pattern(FREM) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_FP_Op::create(parent_bblock, e, frem)->getUID()));
  } else if (s_pattern(TRUNC_OP) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Conv_Op::create(parent_bblock, e, trunc)->getUID()));
  } else if (s_pattern(Z_EXT_OP) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Conv_Op::create(parent_bblock, e, zext)->getUID()));
  } else if (s_pattern(S_EXT_OP) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Conv_Op::create(parent_bblock, e, sext)->getUID()));
  } else if (s_pattern(FP_TRUNC_OP) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Conv_Op::create(parent_bblock, e, fptrunc)->getUID()));
  } else if (s_pattern(FP_EXT_OP) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Conv_Op::create(parent_bblock, e, fpext)->getUID()));
  } else if (s_pattern(FP_TO_UI_OP) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Conv_Op::create(parent_bblock, e, fptoui)->getUID()));
  } else if (s_pattern(FP_TO_SI_OP) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Conv_Op::create(parent_bblock, e, fptosi)->getUID()));
  } else if (s_pattern(UI_TO_FP_OP) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Conv_Op::create(parent_bblock, e, uitofp)->getUID()));
  } else if (s_pattern(SI_TO_FP_OP) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Conv_Op::create(parent_bblock, e, sitofp)->getUID()));
  } else if (s_pattern(PTR_TO_INT_OP) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Conv_Op::create(parent_bblock, e, ptrtoint)->getUID()));
  } else if (s_pattern(INT_TO_PTR_OP) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Conv_Op::create(parent_bblock, e, inttoptr)->getUID()));
  } else if (s_pattern(BITCAST_OP) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Conv_Op::create(parent_bblock, e, bitcast)->getUID()));
  } else if (s_pattern(BITWISE_SHL) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Bitwise_Op::create(parent_bblock, e, _shl)->getUID()));
  } else if (s_pattern(BITWISE_LSHR) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Bitwise_Op::create(parent_bblock, e, _lshr)->getUID()));
  } else if (s_pattern(BITWISE_ASHR) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Bitwise_Op::create(parent_bblock, e, _ashr)->getUID()));
  } else if (s_pattern(BITWISE_AND) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Bitwise_Op::create(parent_bblock, e, _and)->getUID()));
  } else if (s_pattern(BITWISE_OR) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Bitwise_Op::create(parent_bblock, e, _or)->getUID()));
  } else if (s_pattern(BITWISE_XOR) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Bitwise_Op::create(parent_bblock, e, _xor)->getUID()));
  } else if (s_pattern(LOAD_OP) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Load::create(parent_bblock, e)->getUID()));
  } else if (s_pattern(STORE_OP) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Store::create(parent_bblock, e)->getUID()));
  } else if (s_pattern("memset") ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_MemSet::create(parent_bblock, e)->getUID()));
  } else if (s_pattern("memcpy") ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_MemCpy::create(parent_bblock, e)->getUID()));
  } else if (s_pattern("memmove") ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_MemMove::create(parent_bblock, e)->getUID()));
  } else if (s_pattern("va_start") ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_VA_Start::create(parent_bblock, e)->getUID()));
  } else if (s_pattern("va_end") ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_VA_End::create(parent_bblock, e)->getUID()));
  } else if (s_pattern("va_copy") ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_VA_Copy::create(parent_bblock, e)->getUID()));
  } else if (s_pattern(RET_OP) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Return_Value::create(parent_bblock, e)->getUID()));
  } else if (s_pattern(CALL) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Call::create(parent_bblock, e)->getUID()));
  } else if (s_pattern("invoke") ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Invoke::create(parent_bblock, e)->getUID()));
  } else if (s_pattern("abstractvariable") ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Abstract_Variable::create(parent_bblock, e)->getUID()));
  } else if (s_pattern("abstractmemory") ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Abstract_Memory::create(parent_bblock, e)->getUID()));
  } else if (s_pattern(LANDINGPAD) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Landing_Pad::create(parent_bblock, e)->getUID()));
  } else if (s_pattern(RESUME) ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Resume::create(parent_bblock, e)->getUID()));
  } else if (s_pattern("ptrshift") ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Pointer_Shift::create(parent_bblock, e)->getUID()));
  } else if (s_pattern("loadelem") ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Load_Element::create(parent_bblock, e)->getUID()));
  } else if (s_pattern("storeelem") ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Store_Element::create(parent_bblock, e)->getUID()));
  } else if (s_pattern("unreachable") ^ e) {
    stmts.push_back(AR_Node_Ref< AR_Statement >(
        AR_Unreachable::create(parent_bblock, e)->getUID()));
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
  AR_Node_Ref< AR_Operand > operand;

  if (s_pattern("cst") ^ e) {
    operand.setUID(AR_Cst_Operand::create(e)->getUID());
  } else if (s_pattern(VAR_TYPE) ^ e) {
    operand.setUID(AR_Var_Operand::create(e)->getUID());
  } else {
    throw parse_error("ARFactory::createOperand()", e);
  }

  return operand;
}

const std::string ARFactory::CST_BOOL = "constantbool";
const std::string ARFactory::CST_INTEGER = "constantint";
const std::string ARFactory::CST_FLOAT = "constantfp";
const std::string ARFactory::CST_ARRAY = "constantarray";
const std::string ARFactory::CST_STRUCT = "constantstruct";
const std::string ARFactory::CST_POINTER_NULL = "constantpointernull";
const std::string ARFactory::CST_AGGRZERO = "constantaggregatezero";
const std::string ARFactory::GLOBAL_VAR_REF = "globalvariableref";
const std::string ARFactory::LOCAL_VAR_REF = "localvariableref";
const std::string ARFactory::CST_FUNCTION_PTR = "constantfunctionptr";
const std::string ARFactory::CST_UNDEFINED = "undefined";
const std::string ARFactory::LINE = "line";

AR_Node_Ref< AR_Constant > ARFactory::createConstant(s_expression e) {
  std::shared_ptr< AR_Constant > cst;
  if (s_pattern(CST_INTEGER, s_pattern(), s_pattern()) ^ e) {
    std::shared_ptr< AR_Integer_Constant > int_cst(
        AR_Integer_Constant::create(e));
    cst = int_cst;
  } else if (s_pattern(CST_FLOAT, s_pattern(), s_pattern()) ^ e) {
    cst = AR_FP_Constant::create(e);
  } else if (s_pattern(CST_UNDEFINED, s_pattern()) ^ e) {
    cst = AR_Undefined_Constant::create(e);
  } else if (s_pattern(GLOBAL_VAR_REF, s_pattern(), s_pattern(), s_pattern()) ^
             e) {
    cst = AR_Var_Addr_Constant::create(e);
  } else if (s_pattern(LOCAL_VAR_REF) ^ e) {
    cst = AR_Var_Addr_Constant::create(e);
  } else if (s_pattern(CST_FUNCTION_PTR) ^ e) {
    cst = AR_Function_Addr_Constant::create(e);
  } else if (s_pattern("constantpointernull") ^ e) {
    cst = AR_Null_Constant::create(e);
  } else if (s_pattern("range") ^ e) {
    cst = AR_Range_Constant::create(e);
  }

  if (!cst) {
    std::ostringstream ss;
    ss << "Cannot create constant with \"" << e << "\"" << std::endl;
    throw error(ss.str());
  } else {
    AR_Node_Ref< AR_Constant > cst_ref(cst->getUID());
    return cst_ref;
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
  AR_Node_Ref< AR_Global_Variable > gv(AR_Global_Variable::create(e)->getUID());
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
  AR_Node_Ref< AR_Source_Location > src_loc(
      AR_Source_Location::create(e)->getUID());
  return src_loc;
}

AR_Node_Ref< AR_Code > ARFactory::createCode(index64_t parent_function,
                                             s_expression e) {
  std::shared_ptr< AR_Code > ar_code(AR_Code::create(parent_function, e));
  return AR_Node_Ref< AR_Code >(ar_code->getUID());
}

AR_Node_Ref< AR_Function > ARFactory::createFunction(s_expression e) {
  AR_Node_Ref< AR_Function > func(AR_Function::create(e)->getUID());
  ARModel::Instance()->registerFunction(func);
  return func;
}

AR_Node_Ref< AR_Basic_Block > ARFactory::createBasicBlock(index64_t parent_code,
                                                          s_expression e) {
  AR_Node_Ref< AR_Basic_Block > bblock(
      AR_Basic_Block::create(parent_code, e)->getUID());
  return bblock;
}

AR_Node_Ref< AR_Internal_Variable > ARFactory::createInternalVariable(
    s_expression e) {
  // AR_Internal_Variables must be unique by the IDs given from the LLVM
  // ARBOSPass.
  AR_Node_Ref< AR_Code > scope = ARModel::Instance()->getCurrentBuildingScope();
  AR_Node_Ref< AR_Internal_Variable > internal_var(
      AR_Internal_Variable::create(e)->getUID());
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

AR_Node_Ref< AR_Operand > ARFactory::createConstantInteger(
    z_number value, AR_Node_Ref< AR_Integer_Type > type) {
  AR_Node_Ref< AR_Constant > cst(
      AR_Integer_Constant::create(value, type)->getUID());
  AR_Node_Ref< AR_Operand > operand(AR_Cst_Operand::create(cst)->getUID());
  return operand;
}

AR_Node_Ref< AR_Operand > ARFactory::createVarOperand(
    AR_Node_Ref< AR_Internal_Variable > var) {
  AR_Node_Ref< AR_Operand > operand(AR_Var_Operand::create(var)->getUID());
  return operand;
}

ARModel* ARModel::_instance = new ARModel();
