/*******************************************************************************
 *
 * Adaptor for ARBOS IR
 *
 * Author: Jorge A. Navas
 *
 * Contributors: Maxime Arthaud
 *               Clement Decoodt
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

#ifndef ANALYZER_WRAPPER_HPP
#define ANALYZER_WRAPPER_HPP

#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>

#include <arbos/semantics/ar.hpp>
#include <arbos/semantics/fp.hpp>

#include <ikos/number/z_number.hpp>

namespace arbos {

// alias for AR_Null_Ref
const AR_Null_Ref_t Null_ref = {};

// aliases for references to ARBOS Nodes
typedef AR_Node_Ref< ARModel > Model_ref;
typedef AR_Node_Ref< AR_Bundle > Bundle_ref;
typedef AR_Node_Ref< AR_Function > Function_ref;
typedef AR_Node_Ref< AR_Code > Code_ref;
typedef AR_Node_Ref< AR_Basic_Block > Basic_Block_ref;
typedef AR_Node_Ref< AR_Statement > Statement_ref;
typedef AR_Node_Ref< AR_Assignment > Assignment_ref;
typedef AR_Node_Ref< AR_Load > Load_ref;
typedef AR_Node_Ref< AR_Store > Store_ref;
typedef AR_Node_Ref< AR_Extract_Element > Extract_Element_ref;
typedef AR_Node_Ref< AR_Insert_Element > Insert_Element_ref;
typedef AR_Node_Ref< AR_Arith_Op > Arith_Op_ref;
typedef AR_Node_Ref< AR_Integer_Comparison > Integer_Comparison_ref;
typedef AR_Node_Ref< AR_FP_Op > FP_Op_ref;
typedef AR_Node_Ref< AR_FP_Comparison > FP_Comparison_ref;
typedef AR_Node_Ref< AR_Conv_Op > Conv_Op_ref;
typedef AR_Node_Ref< AR_Bitwise_Op > Bitwise_Op_ref;
typedef AR_Node_Ref< AR_Allocate > Allocate_ref;
typedef AR_Node_Ref< AR_Call > Call_ref;
typedef AR_Node_Ref< AR_Invoke > Invoke_ref;
typedef AR_Node_Ref< AR_Return_Value > Return_Value_ref;
typedef AR_Node_Ref< AR_Pointer_Shift > Pointer_Shift_ref;
typedef AR_Node_Ref< AR_Abstract_Variable > Abstract_Variable_ref;
typedef AR_Node_Ref< AR_Abstract_Memory > Abstract_Memory_ref;
typedef AR_Node_Ref< AR_Landing_Pad > Landing_Pad_ref;
typedef AR_Node_Ref< AR_Resume > Resume_ref;
typedef AR_Node_Ref< AR_Unreachable > Unreachable_ref;
typedef AR_Node_Ref< AR_MemCpy > MemCpy_ref;
typedef AR_Node_Ref< AR_MemMove > MemMove_ref;
typedef AR_Node_Ref< AR_MemSet > MemSet_ref;
typedef AR_Node_Ref< AR_VA_Start > VA_Start_ref;
typedef AR_Node_Ref< AR_VA_End > VA_End_ref;
typedef AR_Node_Ref< AR_VA_Arg > VA_Arg_ref;
typedef AR_Node_Ref< AR_VA_Copy > VA_Copy_ref;
typedef AR_Node_Ref< AR_Type > Type_ref;
typedef AR_Node_Ref< AR_Pointer_Type > Pointer_Type_ref;
typedef AR_Node_Ref< AR_Integer_Type > Integer_Type_ref;
typedef AR_Node_Ref< AR_Array_Type > Array_Type_ref;
typedef AR_Node_Ref< AR_Void_Type > Void_Type_ref;
typedef AR_Node_Ref< AR_FP_Type > FP_Type_ref;
typedef AR_Node_Ref< AR_Function_Type > Function_Type_ref;
typedef AR_Node_Ref< AR_Structure_Type > Structure_Type_ref;
typedef AR_Node_Ref< AR_Internal_Variable > Internal_Variable_ref;
typedef AR_Node_Ref< AR_Local_Variable > Local_Variable_ref;
typedef AR_Node_Ref< AR_Global_Variable > Global_Variable_ref;
typedef AR_Node_Ref< AR_Variable > Variable_ref;
typedef AR_Node_Ref< AR_Operand > Operand_ref;
typedef AR_Node_Ref< AR_Var_Operand > Var_Operand_ref;
typedef AR_Node_Ref< AR_Constant > Constant_ref;
typedef AR_Node_Ref< AR_Cst_Operand > Cst_Operand_ref;
typedef AR_Node_Ref< AR_Null_Constant > Null_Constant_ref;
typedef AR_Node_Ref< AR_Integer_Constant > Integer_Constant_ref;
typedef AR_Node_Ref< AR_FP_Constant > FP_Constant_ref;
typedef AR_Node_Ref< AR_Var_Addr_Constant > Var_Addr_Constant_ref;
typedef AR_Node_Ref< AR_Function_Addr_Constant > Function_Addr_Constant_ref;
typedef AR_Node_Ref< AR_Range_Constant > Range_Constant_ref;
typedef AR_Node_Ref< AR_Undefined_Constant > Undefined_ref;
typedef AR_Node_Ref< AR_Source_Location > Source_Location_ref;

// aliases for ARBOS Nodes
typedef ARModel Model;
typedef AR_Bundle Bundle;
typedef AR_Function Function;
typedef AR_Code Code;
typedef AR_Basic_Block Basic_Block;
typedef AR_Statement Statement;
typedef AR_Assignment Assignment;
typedef AR_Load Load;
typedef AR_Store Store;
typedef AR_Extract_Element Extract_Element;
typedef AR_Insert_Element Insert_Element;
typedef AR_Arith_Op Arith_Op;
typedef AR_Integer_Comparison Integer_Comparison;
typedef AR_FP_Op FP_Op;
typedef AR_FP_Comparison FP_Comparison;
typedef AR_Conv_Op Conv_Op;
typedef AR_Bitwise_Op Bitwise_Op;
typedef AR_Allocate Allocate;
typedef AR_Call Call;
typedef AR_Invoke Invoke;
typedef AR_Return_Value Return_Value;
typedef AR_Pointer_Shift Pointer_Shift;
typedef AR_Abstract_Variable Abstract_Variable;
typedef AR_Abstract_Memory Abstract_Memory;
typedef AR_Landing_Pad Landing_Pad;
typedef AR_Resume Resume;
typedef AR_Unreachable Unreachable;
typedef AR_MemCpy MemCpy;
typedef AR_MemMove MemMove;
typedef AR_MemSet MemSet;
typedef AR_VA_Start VA_Start;
typedef AR_VA_End VA_End;
typedef AR_VA_Arg VA_Arg;
typedef AR_VA_Copy VA_Copy;
typedef AR_Type Type;
typedef AR_Pointer_Type Pointer_Type;
typedef AR_Integer_Type Integer_Type;
typedef AR_Array_Type Array_Type;
typedef AR_Void_Type Void_Type;
typedef AR_FP_Type FP_Type;
typedef AR_Function_Type Function_Type;
typedef AR_Structure_Type Structure_Type;
typedef AR_Internal_Variable Internal_Variable;
typedef AR_Local_Variable Local_Variable;
typedef AR_Global_Variable Global_Variable;
typedef AR_Variable Variable;
typedef AR_Operand Operand;
typedef AR_Var_Operand Var_Operand;
typedef AR_Constant Constant;
typedef AR_Cst_Operand Cst_Operand;
typedef AR_Null_Constant Null_Constant;
typedef AR_Integer_Constant Integer_Constant;
typedef AR_FP_Constant FP_Constant;
typedef AR_Var_Addr_Constant Var_Addr_Constant;
typedef AR_Range_Constant Range_Constant;
typedef AR_Function_Addr_Constant Function_Addr_Constant;
typedef AR_Undefined_Constant Undefined;
typedef AR_Source_Location Source_Location;

typedef std::vector< Function_ref > FuncRange;
typedef std::vector< Statement_ref > StmtRange;
typedef std::vector< Basic_Block_ref > BBRange;
typedef std::vector< Global_Variable_ref > GvRange;
typedef std::vector< Local_Variable_ref > LvRange;
typedef std::vector< Internal_Variable_ref > IvRange;
typedef std::vector< Operand_ref > OpRange;

// Wrapper for the arbos Visitor class
class arbos_visitor_api : public Visitor, boost::noncopyable {
protected:
  arbos_visitor_api() {}
  virtual ~arbos_visitor_api() {}

  virtual void visit(Integer_Comparison_ref) = 0;
  virtual void visit(FP_Comparison_ref) = 0;
  virtual void visit(Assignment_ref) = 0;
  virtual void visit(Arith_Op_ref) = 0;
  virtual void visit(Conv_Op_ref) = 0;
  virtual void visit(Bitwise_Op_ref) = 0;
  virtual void visit(FP_Op_ref) = 0;
  virtual void visit(Allocate_ref) = 0;
  virtual void visit(Store_ref) = 0;
  virtual void visit(Load_ref) = 0;
  virtual void visit(Insert_Element_ref) = 0;
  virtual void visit(Extract_Element_ref) = 0;
  virtual void visit(Unreachable_ref) = 0;
  virtual void visit(Landing_Pad_ref) = 0;
  virtual void visit(Resume_ref) = 0;
  virtual void visit(Return_Value_ref) = 0;
  virtual void visit(MemCpy_ref) = 0;
  virtual void visit(MemMove_ref) = 0;
  virtual void visit(MemSet_ref) = 0;
  virtual void visit(Pointer_Shift_ref) = 0;
  virtual void visit(Call_ref) = 0;
  virtual void visit(Invoke_ref) = 0;
  virtual void visit(Abstract_Variable_ref) = 0;
  virtual void visit(Abstract_Memory_ref) = 0;
  virtual void visit(VA_Start_ref) = 0;
  virtual void visit(VA_End_ref) = 0;
  virtual void visit(VA_Arg_ref) = 0;
  virtual void visit(VA_Copy_ref) = 0;

public:
  virtual void visit_start(Basic_Block_ref) {}
  virtual void visit_end(Basic_Block_ref) {}

  virtual void visit_start(Function_ref) {}
  virtual void visit_end(Function_ref) {}

public:
  // We encapsulate AR_Node's into AR_Node_ref's to avoid analyses
  // to have direct access to AR_Node's. AR_Node has a default
  // public copy constructor which doesn't copy properly the state
  // so disastrous results might be produced otherwise.

  void nodeStart(Integer_Comparison& s) {
    visit(AR_Node_Ref< Integer_Comparison >(s.getUID()));
  }
  void nodeStart(FP_Comparison& s) {
    visit(AR_Node_Ref< FP_Comparison >(s.getUID()));
  }
  void nodeStart(Assignment& s) {
    visit(AR_Node_Ref< Assignment >(s.getUID()));
  }
  void nodeStart(Arith_Op& s) { visit(AR_Node_Ref< Arith_Op >(s.getUID())); }
  void nodeStart(Conv_Op& s) { visit(AR_Node_Ref< Conv_Op >(s.getUID())); }
  void nodeStart(Bitwise_Op& s) {
    visit(AR_Node_Ref< Bitwise_Op >(s.getUID()));
  }
  void nodeStart(FP_Op& s) { visit(AR_Node_Ref< FP_Op >(s.getUID())); }
  void nodeStart(Allocate& s) { visit(AR_Node_Ref< Allocate >(s.getUID())); }
  void nodeStart(Store& s) { visit(AR_Node_Ref< Store >(s.getUID())); }
  void nodeStart(Load& s) { visit(AR_Node_Ref< Load >(s.getUID())); }
  void nodeStart(Insert_Element& s) {
    visit(AR_Node_Ref< Insert_Element >(s.getUID()));
  }
  void nodeStart(Extract_Element& s) {
    visit(AR_Node_Ref< Extract_Element >(s.getUID()));
  }
  void nodeStart(Unreachable& s) {
    visit(AR_Node_Ref< Unreachable >(s.getUID()));
  }
  void nodeStart(Landing_Pad& s) {
    visit(AR_Node_Ref< Landing_Pad >(s.getUID()));
  }
  void nodeStart(Resume& s) { visit(AR_Node_Ref< Resume >(s.getUID())); }
  void nodeStart(Return_Value& s) {
    visit(AR_Node_Ref< Return_Value >(s.getUID()));
  }
  void nodeStart(MemCpy& s) { visit(AR_Node_Ref< MemCpy >(s.getUID())); }
  void nodeStart(MemMove& s) { visit(AR_Node_Ref< MemMove >(s.getUID())); }
  void nodeStart(MemSet& s) { visit(AR_Node_Ref< MemSet >(s.getUID())); }
  void nodeStart(Pointer_Shift& s) {
    visit(AR_Node_Ref< Pointer_Shift >(s.getUID()));
  }
  void nodeStart(Call& s) { visit(AR_Node_Ref< Call >(s.getUID())); }
  void nodeStart(Invoke& s) { visit(AR_Node_Ref< Invoke >(s.getUID())); }
  void nodeStart(Abstract_Variable& s) {
    visit(AR_Node_Ref< Abstract_Variable >(s.getUID()));
  }
  void nodeStart(Abstract_Memory& s) {
    visit(AR_Node_Ref< Abstract_Memory >(s.getUID()));
  }
  void nodeStart(VA_Start& s) { visit(AR_Node_Ref< VA_Start >(s.getUID())); }
  void nodeStart(VA_End& s) { visit(AR_Node_Ref< VA_End >(s.getUID())); }
  void nodeStart(VA_Arg& s) { visit(AR_Node_Ref< VA_Arg >(s.getUID())); }
  void nodeStart(VA_Copy& s) { visit(AR_Node_Ref< VA_Copy >(s.getUID())); }

  void nodeStart(Basic_Block& b) {
    visit_start(AR_Node_Ref< Basic_Block >(b.getUID()));
  }
  void nodeEnd(Basic_Block& b) {
    visit_end(AR_Node_Ref< Basic_Block >(b.getUID()));
  }

  void nodeStart(Function& f) {
    visit_start(AR_Node_Ref< Function >(f.getUID()));
  }
  void nodeEnd(Function& f) { visit_end(AR_Node_Ref< Function >(f.getUID())); }

private:
  // DO NOT IMPLEMENT
  virtual void visit(Function_ref) {}
  virtual void visit(Basic_Block_ref) {}

}; // end class arbos_visitor_api

// macro to make explicit dereferences to AR_Node_Ref objects
#define DEREF(X) (*X)

} // end namespace arbos

namespace arbos {

struct location {
  std::string file;
  long long int line;
  long long int column;

  location() : file("?"), line(-1), column(-1) {}
  location(const std::string& f, long long int l, long long int c)
      : file(f), line(l), column(c) {}
};

namespace ar {
/*

  Description: to ensure a layer of separation, ARBOS AR should be
  manipulated only using functions defined in this namespace. By
  doing this if ARBOS AR API changes we would need to update only
  this file.

  Warning: this API should manipulate only AR_Node_ref's never
  AR_Nodes directly since this is not safe.

*/

//! get unique identifier
template < typename Node_ref >
inline unsigned long getUID(const Node_ref& n) {
  return DEREF(n).getUID();
}

//! for applying visitor
template < class Any_ref >
inline void accept(const Any_ref& a, std::shared_ptr< Visitor > vis) {
  DEREF(a).accept(vis);
}

namespace ar_internal {
//! test whether a reference is null or not
template < typename T >
inline bool is_null_ref(const AR_Node_Ref< T >& ref) {
  return ref == Null_ref;
}
//! if operand is a variable
inline bool is_var_operand(const Operand_ref& op) {
  return (DEREF(op).getClassType() == AR_VAR_OPERAND_CLASS_TYPE);
}
//! if operand is a constant
inline bool is_cst_operand(const Operand_ref& op) {
  return (DEREF(op).getClassType() == AR_CST_OPERAND_CLASS_TYPE);
}
//! if constant is an integer
inline bool is_int_constant(const Constant_ref& cst) {
  return (DEREF(cst).getClassType() == AR_INTEGER_CONSTANT_CLASS_TYPE);
}
//! if constant is a float
inline bool is_float_constant(const Constant_ref& cst) {
  return (DEREF(cst).getClassType() == AR_FLOAT_CONSTANT_CLASS_TYPE);
}
//! if constant is null
inline bool is_null_constant(const Constant_ref& cst) {
  return (DEREF(cst).getClassType() == AR_NULL_CONSTANT_CLASS_TYPE);
}
//! if constant is a range
inline bool is_range_constant(const Constant_ref& cst) {
  return (DEREF(cst).getClassType() == AR_RANGE_CONSTANT_CLASS_TYPE);
}
//! if constant is variable address in memory
inline bool is_var_addr_constant(const Constant_ref& cst) {
  return (DEREF(cst).getClassType() == AR_VAR_ADDR_CONSTANT_CLASS_TYPE);
}
//! if operand is variable address in memory
inline bool is_var_addr_constant(const Operand_ref& o) {
  if (!ar_internal::is_cst_operand(o))
    return false;
  Constant_ref cst = DEREF(node_cast< Cst_Operand >(o)).getConstant();
  return is_var_addr_constant(cst);
}
//! return true if the variable is global
inline bool is_global_var(const Variable_ref& v) {
  return (DEREF(v).getClassType() == AR_GLOBAL_VAR_CLASS_TYPE);
}
//! return true if the variable is local
inline bool is_local_var(const Variable_ref& v) {
  return (DEREF(v).getClassType() == AR_LOCAL_VAR_CLASS_TYPE);
}
//! return true if the variable is internal
inline bool is_internal_var(const Variable_ref& v) {
  return (DEREF(v).getClassType() == AR_INTERNAL_VAR_CLASS_TYPE);
}
//! if constant is a function address
inline bool is_function_addr_constant(const Constant_ref& cst) {
  return (DEREF(cst).getClassType() == AR_FUNCTION_ADDR_CONSTANT_CLASS_TYPE);
}
//! if constant is undefined
inline bool is_undefined_constant(const Constant_ref& cst) {
  return (DEREF(cst).getClassType() == AR_UNDEFINED_CONSTANT_CLASS_TYPE);
}
//! if operand is undefined
inline bool is_undefined(const Operand_ref& o) {
  if (ar_internal::is_cst_operand(o)) {
    Constant_ref cst = DEREF(node_cast< Cst_Operand >(o)).getConstant();
    return is_undefined_constant(cst);
  }
  return false;
}
//! if type is a pointer
inline bool is_pointer_type(const Type_ref& type) {
  return (DEREF(type).getClassType() == AR_POINTER_TYPE_CLASS_TYPE);
}
//! if type is void
inline bool is_void_type(const Type_ref& type) {
  return (DEREF(type).getClassType() == AR_VOID_TYPE_CLASS_TYPE);
}
//! if type is struct
inline bool is_struct_type(const Type_ref& type) {
  return (DEREF(type).getClassType() == AR_STRUCT_TYPE_CLASS_TYPE);
}
//! if type is an integer
inline bool is_integer_type(const Type_ref& type) {
  return (DEREF(type).getClassType() == AR_INTEGER_TYPE_CLASS_TYPE);
}
//! if type is an float
inline bool is_float_type(const Type_ref& type) {
  return (DEREF(type).getClassType() == AR_FLOAT_TYPE_CLASS_TYPE);
}
//! if type is an array
inline bool is_array_type(const Type_ref& type) {
  return (DEREF(type).getClassType() == AR_ARRAY_TYPE_CLASS_TYPE);
}
//! helper to get callee address from memory
inline Function_Addr_Constant_ref getFunctionAddr(
    const Call_ref& call) { // pre: call is a direct call.
  Operand_ref op = DEREF(call).getFunction();
  assert(ar_internal::is_cst_operand(op));
  Cst_Operand_ref cst_op = node_cast< Cst_Operand >(op);
  Constant_ref cst = DEREF(cst_op).getConstant();
  Function_Addr_Constant_ref callee = node_cast< Function_Addr_Constant >(cst);
  return callee;
}
//! return true if type is a  function
inline bool is_function_type(const Type_ref& type) {
  return (DEREF(type).getClassType() == AR_FUNCTION_TYPE_CLASS_TYPE);
}

} // end namespace ar_internal

/////
/// SOURCE LOCATIONS
/////

//! get source location of a statement
inline location getSrcLoc(const Source_Location_ref& srcloc) {
  location loc;
  if (!ar_internal::is_null_ref(srcloc)) {
    loc.file = DEREF(srcloc).getFilename();

    std::stringstream o;
    o << DEREF(srcloc).getLineNumber();
    o >> loc.line;

    o.clear();
    o << DEREF(srcloc).getColumnNumber();
    o >> loc.column;
  }
  return loc;
}

template < typename Statement_ref >
inline location getSrcLoc(const Statement_ref& s) {
  return getSrcLoc(DEREF(s).getSourceLocation());
}

template < typename Statement_ref >
inline boost::optional< Function_ref > getParentFunction(
    const Statement_ref& s) {
  Basic_Block_ref bb = DEREF(s).getContainingBasicBlock();
  Code_ref c = DEREF(bb).getContainingCode();
  Function_ref f = DEREF(c).getContainingFunction();
  if (!ar_internal::is_null_ref(f)) {
    return boost::optional< Function_ref >(f);
  } else {
    return boost::optional< Function_ref >();
  }
}

/////
/// BUNDLE
/////

//! get global variables of a module
inline GvRange getGlobalVars(const Bundle_ref& b) {
  assert(!ar_internal::is_null_ref(b));
  return DEREF(b).getGlobalVariables();
}

//! get functions of a module
inline FuncRange getFunctions(const Bundle_ref& b) {
  assert(!ar_internal::is_null_ref(b));
  return DEREF(b).getFunctions();
}

inline boost::optional< Function_ref > getFunction(const Bundle_ref& b,
                                                   const std::string& name) {
  assert(!ar_internal::is_null_ref(b));
  return DEREF(b).getFunctionByNameId(name);
}

//! get target triple
inline const std::string& getTargetTriple(const Bundle_ref& b) {
  assert(!ar_internal::is_null_ref(b));
  return DEREF(b).getTargetTriple();
}

//! get target architecture
inline std::string getTargetArch(const Bundle_ref& b) {
  assert(!ar_internal::is_null_ref(b));
  std::string target_triple = DEREF(b).getTargetTriple();
  return target_triple.substr(0, target_triple.find('-'));
}

///////
/// Global variables
///////
inline Code_ref getInitializer(Global_Variable_ref gv) {
  assert(!ar_internal::is_null_ref(gv));
  return DEREF(gv).getInitializer();
}

///////
/// FUNCTION BODY
///////

//! entry block of the function
inline Basic_Block_ref getEntryBlock(const Code_ref& c) {
  assert(!ar_internal::is_null_ref(c));
  Basic_Block_ref bb = DEREF(c).getEntryBlock();
  assert(!ar_internal::is_null_ref(bb));
  return bb;
}
//! exit block of the function
inline boost::optional< Basic_Block_ref > getExitBlock(const Code_ref& c) {
  assert(!ar_internal::is_null_ref(c));
  Basic_Block_ref bb = DEREF(c).getExitBlock();
  if (ar_internal::is_null_ref(bb))
    return boost::optional< Basic_Block_ref >();
  else
    return boost::optional< Basic_Block_ref >(bb);
}
//! unreachable block of the function
inline boost::optional< Basic_Block_ref > getUnreachableBlock(
    const Code_ref& c) {
  assert(!ar_internal::is_null_ref(c));
  Basic_Block_ref bb = DEREF(c).getUnreachableBlock();
  if (ar_internal::is_null_ref(bb))
    return boost::optional< Basic_Block_ref >();
  else
    return boost::optional< Basic_Block_ref >(bb);
}
//! ehresume block of the function
inline boost::optional< Basic_Block_ref > getEHResumeBlock(const Code_ref& c) {
  assert(!ar_internal::is_null_ref(c));
  Basic_Block_ref bb = DEREF(c).getEHResumeBlock();
  if (ar_internal::is_null_ref(bb))
    return boost::optional< Basic_Block_ref >();
  else
    return boost::optional< Basic_Block_ref >(bb);
}
//! local variables of the function
inline LvRange getLocalVars(const Function_ref& f) {
  return DEREF(f).getLocalVariables();
}
//! Internal variables of the function (e.g. LLVM registers)
inline IvRange getInternalVars(const Code_ref& c) {
  assert(!ar_internal::is_null_ref(c));
  return DEREF(c).getInternalVariables();
}
//! blocks of the function
inline BBRange getBlocks(const Code_ref& c) {
  assert(!ar_internal::is_null_ref(c));
  return DEREF(c).getBlocks();
}
//! return a reference to a block
inline Basic_Block_ref getBlockRef(const Code_ref& c, std::string bb_name) {
  assert(!ar_internal::is_null_ref(c));
  return DEREF(c).getBasicBlockByNameId(bb_name);
}

///////
/// FUNCTIONS
///////

//! function body
inline Code_ref getBody(const Function_ref& f) {
  assert(!ar_internal::is_null_ref(f));
  return DEREF(f).getFunctionBody();
}
//! function return type
inline Type_ref getReturnType(const Function_ref& f) {
  assert(!ar_internal::is_null_ref(f));
  return DEREF(f).getReturnType();
}
//! function formal parameters
inline IvRange getFormalParams(const Function_ref& f) {
  assert(!ar_internal::is_null_ref(f));
  return DEREF(f).getFormalParams();
}
//! entry block of the function
inline Basic_Block_ref getEntryBlock(const Function_ref& f) {
  return getEntryBlock(getBody(f));
}
//! exit block of the function
inline boost::optional< Basic_Block_ref > getExitBlock(const Function_ref& f) {
  return getExitBlock(getBody(f));
}
//! all the blocks of the function
inline BBRange getBlocks(const Function_ref& f) {
  Code_ref b = getBody(f);
  assert(!ar_internal::is_null_ref(b));
  return DEREF(b).getBlocks();
}
//! true if the function has a variable number of arguments foo(...)
inline bool isVarargs(const Function_ref& f) {
  assert(!ar_internal::is_null_ref(f));
  Function_Type_ref ty = DEREF(f).getFunctionType();
  assert(!ar_internal::is_null_ref(ty));
  return DEREF(ty).isVarargs();
}

///////
/// BASIC BLOCKS
///////

//! statements of a block
inline StmtRange getStatements(const Basic_Block_ref& b) {
  assert(!ar_internal::is_null_ref(b));
  return DEREF(b).getStatements();
}
//! successor blocks
inline BBRange getSuccs(const Basic_Block_ref& b) {
  assert(!ar_internal::is_null_ref(b));
  std::pair< bblock_iterator, bblock_iterator > next_blocks =
      DEREF(b).getNextBlocks();
  BBRange succs;
  std::copy(next_blocks.first, next_blocks.second, std::back_inserter(succs));
  return succs;
}
//! predecessor blocks
inline BBRange getPreds(const Basic_Block_ref& b) {
  assert(!ar_internal::is_null_ref(b));
  std::pair< bblock_iterator, bblock_iterator > prev_blocks =
      DEREF(b).getPreviousBlocks();
  BBRange preds;
  std::copy(prev_blocks.first, prev_blocks.second, std::back_inserter(preds));
  return preds;
}
//! return the function body that contains the block
inline Code_ref getParent(const Basic_Block_ref& b) {
  assert(!ar_internal::is_null_ref(b));
  return DEREF(b).getContainingCode();
}

///////
/// Binary statements: arithmetic, fp, bitwise
///////

//! return lhs of the binary operation
template < typename Binary_Statement_ref >
inline Internal_Variable_ref getResult(const Binary_Statement_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getResult();
}
//! return the first operator on the rhs of the binary operation
template < typename Binary_Statement_ref >
inline Operand_ref getLeftOp(const Binary_Statement_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getLeftOperand();
}
//! return the second operator on the rhs of the binary operation
template < typename Binary_Statement_ref >
inline Operand_ref getRightOp(const Binary_Statement_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getRightOperand();
}
//! return code number of the arithmetic operation
inline ArithOp getArithOp(const Arith_Op_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getArithOp();
}
//! return code number of the floating point operation
inline FPOp getFPOp(const FP_Op_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getFPOp();
}
//! return code number of the bitwise operation
inline BitwiseOp getBitwiseOp(const Bitwise_Op_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getBitwiseOp();
}

///////
/// Integer comparisons
///////

//! return the first operand of the comparison
inline Operand_ref getLeftOp(const Integer_Comparison_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getLeftOperand();
}
//! return the second operand of the comparison
inline Operand_ref getRightOp(const Integer_Comparison_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getRightOperand();
}
//! return the code number of the comparison operator
inline CompOp getPredicate(const Integer_Comparison_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getPredicate();
}
//! if false then the meaning of the comparison is the negation of
//! its predicate
inline bool isPositive(const Integer_Comparison_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).isContinueIfTrue();
}

///////
/// FP comparisons
///////

//! return the first operand of the comparison
inline Operand_ref getLeftOp(const FP_Comparison_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getLeftOperand();
}
//! return the second operand of the comparison
inline Operand_ref getRightOp(const FP_Comparison_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getRightOperand();
}
//! return the code number of the comparison operator
inline FP_CompOp getPredicate(const FP_Comparison_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getPredicate();
}
//! if false then the meaning of the comparison is the negation of
//! its predicate
inline bool isPositive(const FP_Comparison_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).isContinueIfTrue();
}

///////
/// Assignments
///////

//! return the lhs of the assignment
inline Internal_Variable_ref getLeftOp(const Assignment_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getLeftOperand();
}
//! return the rhs of the assignment
inline Operand_ref getRightOp(const Assignment_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getRightOperand();
}

///////
/// Casting operators
///////

//! return the lhs of the conversion operator
inline Operand_ref getLeftOp(const Conv_Op_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getResult();
}
//! return the rhs of the conversion operator
inline Operand_ref getRightOp(const Conv_Op_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getOperand();
}
//! return the code number of the conversion operator
inline ConvOp getConvOp(const Conv_Op_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getConvOp();
}

///////
/// Allocate
///////

//! return the address of the local variable
inline Var_Addr_Constant_ref getResult(const Allocate_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getLocalVariableAddress();
}
//! returns the type of the elements
inline Type_ref getAllocatedType(const Allocate_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getAllocatedType();
}
//! returns number of elements in this array structure
inline Operand_ref getArraySize(const Allocate_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getArraySize();
}

///////
/// Memory load
///////

//! return lhs of the load instruction
inline Internal_Variable_ref getResult(const Load_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getResult();
}
//! return the pointer of the load instruction
inline Operand_ref getPointer(const Load_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getPointer();
}

///////
/// Memory store
///////

//! return the pointer of the store instruction
inline Operand_ref getPointer(const Store_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getPointer();
}
//! return the value to be stored
inline Operand_ref getValue(const Store_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getValue();
}

///////
/// Extract Element
///////

//! return lhs of the extract element instruction
inline Internal_Variable_ref getResult(const Extract_Element_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getResult();
}
//! return the aggregate of the extract element instruction
inline Operand_ref getAggregate(const Extract_Element_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getValue();
}
//! return the offset of the extract element instruction
inline Operand_ref getOffset(const Extract_Element_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getOffset();
}

///////
/// Insert Element
///////

//! return lhs of the insert element instruction
inline Internal_Variable_ref getResult(const Insert_Element_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getResult();
}
//! return the aggregate of the insert element instruction
inline Operand_ref getAggregate(const Insert_Element_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getValue();
}
//! return the offset of the insert element instruction
inline Operand_ref getOffset(const Insert_Element_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getOffset();
}
//! return the stored element of the insert element instruction
inline Operand_ref getElement(const Insert_Element_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getElement();
}

///////
/// Pointer arithmetic
///////

//! return the lhs of the pointer arithmetic calculation
inline Internal_Variable_ref getResult(const Pointer_Shift_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getResult();
}
//! return the base of the pointer arithmetic calculation
inline Operand_ref getBase(const Pointer_Shift_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getPointer();
}
//! return the offset of the pointer arithmetic calculation
inline Operand_ref getOffset(const Pointer_Shift_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getOffset();
}

///////
/// Memory intrinsics: memcpy
///////

//! return the src of the memcpy
inline Operand_ref getSource(const MemCpy_ref& s) {
  assert(!ar_internal::is_null_ref(s));
  return DEREF(s).getSource();
}
//! return the destination of the memcpy
inline Operand_ref getTarget(const MemCpy_ref& s) {
  assert(!ar_internal::is_null_ref(s));
  return DEREF(s).getTarget();
}
//! return the number of bytes to be copied
inline Operand_ref getLen(const MemCpy_ref& s) {
  assert(!ar_internal::is_null_ref(s));
  return DEREF(s).getLength();
}

///////
/// Memory intrinsics: memmove
///////

//! return the src of the memmove
inline Operand_ref getSource(const MemMove_ref& s) {
  assert(!ar_internal::is_null_ref(s));
  return DEREF(s).getSource();
}
//! return the destination of the memmove
inline Operand_ref getTarget(const MemMove_ref& s) {
  assert(!ar_internal::is_null_ref(s));
  return DEREF(s).getTarget();
}
//! return the number of bytes to be copied
inline Operand_ref getLen(const MemMove_ref& s) {
  assert(!ar_internal::is_null_ref(s));
  return DEREF(s).getLength();
}

///////
/// Memory intrinsics: memset
///////

//! return the base pointer of the memset
inline Operand_ref getBase(const MemSet_ref& s) {
  assert(!ar_internal::is_null_ref(s));
  return DEREF(s).getOperand();
}
//! return the value to be copied
inline Operand_ref getValue(const MemSet_ref& s) {
  assert(!ar_internal::is_null_ref(s));
  return DEREF(s).getValue();
}
//! return the number of bytes to be copied
inline Operand_ref getLen(const MemSet_ref& s) {
  assert(!ar_internal::is_null_ref(s));
  return DEREF(s).getLength();
}

///////
/// Special instruction for abstracting a variable
///////

//! return the variable to be abstracted
inline Internal_Variable_ref getVar(const Abstract_Variable_ref& s) {
  assert(!ar_internal::is_null_ref(s));
  return DEREF(s).getVariable();
}

///////
/// Special instruction for abstracting a memory location
///////

//! return the memory location to be abstracted
inline Operand_ref getPointer(const Abstract_Memory_ref& s) {
  assert(!ar_internal::is_null_ref(s));
  return DEREF(s).get_pointer();
}
//! return the number of bytes to be abstracted
inline Operand_ref getLen(const Abstract_Memory_ref& s) {
  assert(!ar_internal::is_null_ref(s));
  return DEREF(s).get_length();
}

///////
/// Return value of a function
///////

//! return value of the a function if exists
inline boost::optional< Operand_ref > getReturnValue(
    const Return_Value_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  if (ar_internal::is_null_ref(DEREF(r).getValue()))
    return boost::optional< Operand_ref >();
  else
    return boost::optional< Operand_ref >(DEREF(r).getValue());
}

///////
/// Landing pad
///////

//! return the variable containing the exception
inline Internal_Variable_ref getVar(const Landing_Pad_ref& s) {
  assert(!ar_internal::is_null_ref(s));
  return DEREF(s).getException();
}

///////
/// Resume
///////

//! return the variable containing the exception
inline Internal_Variable_ref getVar(const Resume_ref& s) {
  assert(!ar_internal::is_null_ref(s));
  return DEREF(s).getException();
}

//////
/// Variable Arguments
//////

// return the pointer of the va_start
inline Operand_ref getPointer(const VA_Start_ref& s) {
  assert(!ar_internal::is_null_ref(s));
  return DEREF(s).getPointer();
}
// return the pointer of the va_end
inline Operand_ref getPointer(const VA_End_ref& s) {
  assert(!ar_internal::is_null_ref(s));
  return DEREF(s).getPointer();
}
// return the lhs of the va_arg
inline Internal_Variable_ref getResult(const VA_Arg_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getResult();
}
// return the pointer of the va_arg
inline Operand_ref getPointer(const VA_Arg_ref& s) {
  assert(!ar_internal::is_null_ref(s));
  return DEREF(s).getPointer();
}
//! return the src of the va_copy
inline Operand_ref getSource(const VA_Copy_ref& s) {
  assert(!ar_internal::is_null_ref(s));
  return DEREF(s).getSource();
}
//! return the destination of the va_copy
inline Operand_ref getDestination(const VA_Copy_ref& s) {
  assert(!ar_internal::is_null_ref(s));
  return DEREF(s).getDestination();
}

//////
/// Call sites
//////

//! if callee function is indirect
inline bool isIndirectCall(const Call_ref& call) {
  assert(!ar_internal::is_null_ref(call));
  Operand_ref op = DEREF(call).getFunction();
  return ar_internal::is_var_operand(op);
}

//! if callee function is direct
inline bool isDirectCall(const Call_ref& call) {
  assert(!ar_internal::is_null_ref(call));
  Operand_ref op = DEREF(call).getFunction();
  return ar_internal::is_cst_operand(op);
}

//! return var that refers to the indirect function call
inline Internal_Variable_ref getIndirectCallVar(const Call_ref& call) {
  assert(!ar_internal::is_null_ref(call));
  Operand_ref op = DEREF(call).getFunction();
  assert(ar_internal::is_var_operand(op));
  Var_Operand_ref var_op = node_cast< Var_Operand >(op);
  return DEREF(var_op).getInternalVariable();
}

//! get actual parameters of the call
inline OpRange getArguments(const Call_ref& cs) {
  assert(!ar_internal::is_null_ref(cs));
  return DEREF(cs).getArguments();
}
//! return the return value of the call site if exists
inline boost::optional< Internal_Variable_ref > getReturnValue(
    const Call_ref& cs) {
  assert(!ar_internal::is_null_ref(cs));
  if (ar_internal::is_null_ref(DEREF(cs).getReturnValue()))
    return boost::optional< Internal_Variable_ref >();
  else
    return boost::optional< Internal_Variable_ref >(DEREF(cs).getReturnValue());
}

//! get callee function
inline Function_ref getFunction(const Call_ref& call) {
  assert(!ar::isIndirectCall(call));
  Function_Addr_Constant_ref callee = ar_internal::getFunctionAddr(call);
  assert(!ar_internal::is_null_ref(callee));
  Function_ref res = DEREF(callee).getFunction();
  return res;
}
//! get name of the callee function
inline std::string getFunctionName(const Function_Addr_Constant_ref& callee) {
  assert(!ar_internal::is_null_ref(callee));
  return DEREF(callee).getFunctionName();
}
//! get name of the callee function
inline std::string getFunctionName(const Call_ref& call) {
  assert(ar::isDirectCall(call));
  Function_Addr_Constant_ref callee = ar_internal::getFunctionAddr(call);
  return getFunctionName(callee);
}
//! if callee function is external
inline bool isExternal(const Function_ref& f) {
  return ar_internal::is_null_ref< Function >(f) ||
         ar_internal::is_null_ref< Code >(ar::getBody(f));
}
inline bool isExternal(const Call_ref& call) {
  assert(ar::isDirectCall(call));
  Function_ref f = getFunction(call);
  return isExternal(f);
}

inline Call_ref getFunctionCall(Invoke_ref invoke) {
  assert(!ar_internal::is_null_ref(invoke));
  return DEREF(invoke).getFunctionCall();
}

///////
/// get methods for names
///////

//! name of the function
inline std::string getName(const Function_ref& f) {
  assert(!ar_internal::is_null_ref(f));
  return DEREF(f).getFunctionId();
}
//! name of the block
inline std::string getName(const Basic_Block_ref& b) {
  assert(!ar_internal::is_null_ref(b));
  return DEREF(b).getNameId();
}
//! name of the internal variable
inline std::string getName(const Internal_Variable_ref& v) {
  assert(!ar_internal::is_null_ref(v));
  return DEREF(v).getName();
}
//! name of the global variable
inline std::string getName(const Global_Variable_ref& v) {
  assert(!ar_internal::is_null_ref(v));
  return DEREF(v).getNameId();
}
//! name of the local variable
inline std::string getName(const Local_Variable_ref& v) {
  assert(!ar_internal::is_null_ref(v));
  return DEREF(v).getNameId();
}
//! name of the variable operand
inline std::string getName(const Var_Operand_ref& v) {
  assert(!ar_internal::is_null_ref(v));
  return getName(DEREF(v).getInternalVariable());
}
//! name of the variable
inline std::string getName(const Variable_ref& v) {
  assert(!ar_internal::is_null_ref(v));
  return DEREF(v).getNameId();
}
//! name of a variable address constant
inline std::string getName(const Var_Addr_Constant_ref& v) {
  assert(!ar_internal::is_null_ref(v));
  return getName(DEREF(v).getVariable());
}
//! name of the variable to be abstracted
inline std::string getName(const Abstract_Variable_ref& s) {
  return getName(getVar(s));
}

///////
/// get methods for types
///////

//! type of the internal variable
inline Type_ref getType(const Internal_Variable_ref& v) {
  assert(!ar_internal::is_null_ref(v));
  return DEREF(v).getType();
}
//! type of the global variable
inline Type_ref getType(const Global_Variable_ref& gv) {
  assert(!ar_internal::is_null_ref(gv));
  return DEREF(gv).getType();
}
//! type of a local variable
inline Type_ref getType(const Local_Variable_ref& lv) {
  assert(!ar_internal::is_null_ref(lv));
  return DEREF(lv).getType();
}
//! type of the variable operand
inline Type_ref getType(const Var_Operand_ref& v) {
  assert(!ar_internal::is_null_ref(v));
  return DEREF(DEREF(v).getInternalVariable()).getType();
}
//! type of the pointee
inline Type_ref getPointeeType(const Pointer_Type_ref& t) {
  assert(!ar_internal::is_null_ref(t));
  return DEREF(t).getCellType();
}
inline Type_ref getPointeeType(const Type_ref& ty) {
  assert(!ar_internal::is_null_ref(ty));
  assert(ar_internal::is_pointer_type(ty));
  Pointer_Type_ref p_ty = node_cast< Pointer_Type >(ty);
  return getPointeeType(p_ty);
}
inline Type_ref getCellType(const Array_Type_ref& t) {
  assert(!ar_internal::is_null_ref(t));
  return DEREF(t).getCellType();
}
inline Type_ref getCellType(const Type_ref& ty) {
  assert(!ar_internal::is_null_ref(ty));
  assert(ar_internal::is_array_type(ty));
  Array_Type_ref p_ty = node_cast< Array_Type >(ty);
  return getCellType(p_ty);
}

//! type of the variable
inline Type_ref getType(const Variable_ref& v) {
  assert(!ar_internal::is_null_ref(v));
  return DEREF(v).getType();
}
//! type of the variable address constant
inline Type_ref getType(const Var_Addr_Constant_ref& v) {
  assert(!ar_internal::is_null_ref(v));
  return getType(DEREF(v).getVariable());
}
//! type of an undefined constant
inline Type_ref getType(const Undefined_ref& v) {
  assert(!ar_internal::is_null_ref(v));
  return DEREF(v).getType();
}
//! type of a range
inline Type_ref getType(const Range_Constant_ref& v) {
  assert(!ar_internal::is_null_ref(v));
  return DEREF(v).getType();
}

//! type of a constant
inline Type_ref getType(const Constant_ref& cst) {
  if (ar_internal::is_var_addr_constant(cst)) {
    Var_Addr_Constant_ref v = node_cast< Var_Addr_Constant >(cst);
    return getType(DEREF(v).getVariable());
  } else if (ar_internal::is_int_constant(cst)) {
    Integer_Constant_ref n = node_cast< Integer_Constant >(cst);
    return node_cast< Type >(DEREF(n).getType());
  } else if (ar_internal::is_float_constant(cst)) {
    FP_Constant_ref n = node_cast< FP_Constant >(cst);
    return node_cast< Type >(DEREF(n).getType());
  } else if (ar_internal::is_function_addr_constant(cst)) {
    Function_Addr_Constant_ref func_addr =
        node_cast< Function_Addr_Constant >(cst);
    return node_cast< Type >(DEREF(func_addr).getType());
  } else if (ar_internal::is_null_constant(cst)) {
    Null_Constant_ref null = node_cast< Null_Constant >(cst);
    return node_cast< Type >(DEREF(null).getType());
  } else if (ar_internal::is_undefined_constant(cst)) {
    Undefined_ref undefined = node_cast< Undefined >(cst);
    return node_cast< Type >(DEREF(undefined).getType());
  } else if (ar_internal::is_range_constant(cst)) {
    Range_Constant_ref range = node_cast< Range_Constant >(cst);
    return node_cast< Type >(DEREF(range).getType());
  } else {
    throw error("getType: unexpected constant operand");
  }
}

//! type of the operand
inline Type_ref getType(const Operand_ref& o) {
  // A bit expensive: we need to dig in the operand to find out
  // the type
  if (ar_internal::is_var_operand(o)) {
    return getType(node_cast< Var_Operand >(o));
  } else if (ar_internal::is_cst_operand(o)) {
    Cst_Operand_ref cst_operand = node_cast< Cst_Operand >(o);
    return getType(DEREF(cst_operand).getConstant());
  } else {
    throw error("getType: unexpected operand");
  }
}

///////
/// get methods for alignment
///////

//! return the alignment of a global variable in bytes, or boost::none
inline boost::optional< z_number > getAlignment(const Global_Variable_ref& gv) {
  assert(!ar_internal::is_null_ref(gv));
  return DEREF(gv).getAlignment();
}

//! return the alignment of a local variable in bytes, or boost::none
inline boost::optional< z_number > getAlignment(const Local_Variable_ref& lv) {
  assert(!ar_internal::is_null_ref(lv));
  return DEREF(lv).getAlignment();
}

//! return the alignment of a load instruction in bytes, or boost::none
inline boost::optional< z_number > getAlignment(const Load_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getAlignment();
}

//! return the alignment of a store instruction in bytes, or boost::none
inline boost::optional< z_number > getAlignment(const Store_ref& r) {
  assert(!ar_internal::is_null_ref(r));
  return DEREF(r).getAlignment();
}

//! return the alignment of a memcpy instruction in bytes, or boost::none
inline boost::optional< z_number > getAlignment(const MemCpy_ref& s) {
  assert(!ar_internal::is_null_ref(s));
  return DEREF(s).getAlignment();
}

//! return the alignment of a memmove instruction in bytes, or boost::none
inline boost::optional< z_number > getAlignment(const MemMove_ref& s) {
  assert(!ar_internal::is_null_ref(s));
  return DEREF(s).getAlignment();
}

//! return the alignment of a memset instruction in bytes, or boost::none
inline boost::optional< z_number > getAlignment(const MemSet_ref& s) {
  assert(!ar_internal::is_null_ref(s));
  return DEREF(s).getAlignment();
}

///////
/// get methods for sizes
///////

//! return the size of a pointer in bytes
inline ikos::z_number getPointerSize(const Bundle_ref& b) {
  arbos::z_number sz = DEREF(b).get_sizeofptr();
  return ikos::z_number(static_cast< mpz_class >(sz));
}

//! size of an arbitrary type in bytes
inline ikos::z_number getSize(const Type_ref& t) {
  assert(!ar_internal::is_null_ref(t));
  arbos::z_number sz = DEREF(t).getStoreSize();
  return ikos::z_number(static_cast< mpz_class >(sz));
}

//! bitwidth of an integer in bytes
inline ikos::z_number getSize(const Integer_Constant_ref& n) {
  assert(!ar_internal::is_null_ref(n));
  return getSize(DEREF(n).getType());
}

//! size of an arbitrary type in bits
inline uint64_t getRealSize(const Type_ref& t) {
  assert(!ar_internal::is_null_ref(t));
  mpz_class n = static_cast< mpz_class >(DEREF(t).getRealSize());
  if (n.fits_ulong_p()) {
    return static_cast< uint64_t >(n.get_ui());
  } else {
    std::ostringstream buf;
    buf << n;
    return boost::lexical_cast< uint64_t >(buf.str());
  }
}

///////
/// get methods for allocated sizes
///////

//! Return the size in bytes.
//  The returned size is extracted from the AR type.
inline ikos::z_number getAllocatedSize(const Type_ref& type) {
  if (ar_internal::is_pointer_type(type)) {
    Pointer_Type_ref ptr_type = node_cast< AR_Pointer_Type >(type);
    return getSize(ar::getPointeeType(ptr_type));
  } else {
    return getSize(type);
  }
}

//! Return the allocated size (in bytes) by the global or alloca
//  The returned size is extracted from the AR type.
inline ikos::z_number getAllocatedSize(const Operand_ref& o) {
  if (ar_internal::is_cst_operand(o)) {
    Cst_Operand_ref cst_o = node_cast< AR_Cst_Operand >(o);
    assert(!ar_internal::is_null_ref(cst_o));
    Constant_ref cst = DEREF(cst_o).getConstant();
    if (ar_internal::is_var_addr_constant(cst)) {
      Var_Addr_Constant_ref var_cst = node_cast< Var_Addr_Constant >(cst);
      return getAllocatedSize(getType(var_cst));
    } else if (ar_internal::is_function_addr_constant(cst)) {
      return ikos::z_number(1);
    }
  }
  throw error("getAllocatedSize: cannot determine allocated size");
}

///////
/// get methods for values
///////

inline z_number getZValue(const Integer_Constant_ref& n) {
  assert(!ar_internal::is_null_ref(n));
  return DEREF(n).getValue();
}

//! Value of an fp constant
inline std::string getFPValueStr(const FP_Constant_ref& n) {
  std::ostringstream buf;
  assert(!ar_internal::is_null_ref(n));
  fp_number k = DEREF(n).getValue();
  buf << k;
  return buf.str();
}

inline fp_number getFPValue(const FP_Constant_ref& n) {
  assert(!ar_internal::is_null_ref(n));
  return DEREF(n).getValue();
}

//! Constant associated to the operand
inline Constant_ref getConstant(const Cst_Operand_ref& o) {
  assert(!ar_internal::is_null_ref(o));
  return DEREF(o).getConstant();
}

///////
/// Queries
///////

//! return true if the operand is a pointer
inline bool isPointer(const Operand_ref& o) {
  return ar_internal::is_pointer_type(getType(o));
}
//! return true if the variable is a pointer
inline bool isPointer(const Variable_ref& v) {
  return ar_internal::is_pointer_type(getType(v));
}
//! return true if the variable operand is a pointer
inline bool isPointer(const Var_Operand_ref& v) {
  return ar_internal::is_pointer_type(getType(v));
}
//! return true if the variable address constant is a pointer
inline bool isPointer(const Var_Addr_Constant_ref& v) {
  return isPointer(DEREF(v).getVariable());
}
//! return true if the internal variable is a pointer
inline bool isPointer(const Internal_Variable_ref& v) {
  return ar_internal::is_pointer_type(getType(v));
}
//! return true if the global variable is a pointer
inline bool isPointer(const Global_Variable_ref& gv) {
  return ar_internal::is_pointer_type(getType(gv));
}

//! return true if operand is a variable which was lowered to a
//! register by llvm (internal variables and formal parameters)
inline bool isRegVar(const Operand_ref& o) {
  return ar_internal::is_var_operand(o);
}

//! return true if operand is a variable lowered to an integer
//! register (internal variables and formal parameters)
inline bool isIntegerRegVar(const Operand_ref& o) {
  if (isRegVar(o)) {
    Var_Operand_ref v = node_cast< Var_Operand >(o);
    return ar_internal::is_integer_type(ar::getType(v));
  }
  return false;
}

//! return true if o is a global variable
inline bool isGlobalVar(const Operand_ref& o) {
  if (!ar_internal::is_cst_operand(o))
    return false;

  Cst_Operand_ref cst_o = node_cast< Cst_Operand >(o);
  Constant_ref cst = DEREF(cst_o).getConstant();

  if (!ar_internal::is_var_addr_constant(cst))
    return false;

  Var_Addr_Constant_ref var_addr = node_cast< Var_Addr_Constant >(cst);
  Variable_ref var = DEREF(var_addr).getVariable();
  return ar_internal::is_global_var(var);
}

//! return true if o is a stack variable
inline bool isAllocaVar(const Operand_ref& o) {
  if (!ar_internal::is_cst_operand(o))
    return false;

  Cst_Operand_ref cst_o = node_cast< Cst_Operand >(o);
  Constant_ref cst = DEREF(cst_o).getConstant();

  if (!ar_internal::is_var_addr_constant(cst))
    return false;

  Var_Addr_Constant_ref var_addr = node_cast< Var_Addr_Constant >(cst);
  Variable_ref var = DEREF(var_addr).getVariable();
  return ar_internal::is_local_var(var);
}

//! return true if o is a function pointer
inline bool isFunctionPointer(const Operand_ref& o) {
  if (!ar_internal::is_cst_operand(o))
    return false;

  Cst_Operand_ref cst_o = node_cast< Cst_Operand >(o);
  Constant_ref cst = DEREF(cst_o).getConstant();

  return ar_internal::is_function_addr_constant(cst);
}

//! return true if the internal variable is an integer
inline bool isInteger(const Internal_Variable_ref& v) {
  return (ar_internal::is_integer_type(ar::getType(v)));
}

//! return true if the internal variable is void
inline bool isVoid(const Internal_Variable_ref& v) {
  return (ar_internal::is_void_type(ar::getType(v)));
}

//! return true if the internal variable is a struct
inline bool isStruct(const Internal_Variable_ref& v) {
  return (ar_internal::is_struct_type(ar::getType(v)));
}

//! return true if the type is an integer
inline bool isInteger(const Type_ref& ty) {
  return (ar_internal::is_integer_type(ty));
}

//! return true if the type is a float
inline bool isFloat(const Type_ref& ty) {
  return (ar_internal::is_float_type(ty));
}

//! return true if the type is a pointer
inline bool isPointer(const Type_ref& ty) {
  return (ar_internal::is_pointer_type(ty));
}

//! return true if the type is a function
inline bool isFunction(const Type_ref& ty) {
  return (ar_internal::is_function_type(ty));
}

//! return true if the type is an array
inline bool isArray(const Type_ref& ty) {
  return (ar_internal::is_array_type(ty));
}

inline boost::optional< Global_Variable_ref > getGlobalVariable(
    const Operand_ref& o) {
  if (!ar_internal::is_cst_operand(o))
    return boost::optional< Global_Variable_ref >();

  Cst_Operand_ref cst_o = node_cast< Cst_Operand >(o);
  Constant_ref cst = DEREF(cst_o).getConstant();

  if (!ar_internal::is_var_addr_constant(cst))
    return boost::optional< Global_Variable_ref >();

  Var_Addr_Constant_ref var_addr = node_cast< Var_Addr_Constant >(cst);
  Variable_ref var = DEREF(var_addr).getVariable();
  if (!ar_internal::is_global_var(var))
    return boost::optional< Global_Variable_ref >();

  Global_Variable_ref gvar = node_cast< Global_Variable >(var);
  return boost::optional< Global_Variable_ref >(gvar);
}

/////
//! To test membership of statements
/////

inline bool is_int_arith_stmt(const Statement_ref& s) {
  return (DEREF(s).getClassType() == AR_INTEGER_ARITH_OP_STATEMENT_CLASS_TYPE);
}

inline bool is_float_arith_stmt(const Statement_ref& s) {
  return (DEREF(s).getClassType() == AR_FP_ARITH_OP_STATEMENT_CLASS_TYPE);
}

inline bool is_int_cmp_stmt(const Statement_ref& s) {
  return (DEREF(s).getClassType() ==
          AR_INTEGER_COMPARISON_STATEMENT_CLASS_TYPE);
}

inline bool is_float_cmp_stmt(const Statement_ref& s) {
  return (DEREF(s).getClassType() == AR_FP_COMPARISON_STATEMENT_CLASS_TYPE);
}

inline bool is_assignment_stmt(const Statement_ref& s) {
  return (DEREF(s).getClassType() == AR_ASSIGNMENT_STATEMENT_CLASS_TYPE);
}

inline bool is_conv_stmt(const Statement_ref& s) {
  return (DEREF(s).getClassType() == AR_CONV_OP_STATEMENT_CLASS_TYPE);
}

inline bool is_bitwise_stmt(const Statement_ref& s) {
  return (DEREF(s).getClassType() == AR_BITWISE_OP_STATEMENT_CLASS_TYPE);
}

inline bool is_call_stmt(const Statement_ref& s) {
  return (DEREF(s).getClassType() == AR_CALL_STATEMENT_CLASS_TYPE);
}

inline bool is_return_stmt(const Statement_ref& s) {
  return (DEREF(s).getClassType() == AR_RETURN_VALUE_STATEMENT_CLASS_TYPE);
}

inline bool is_allocate_stmt(const Statement_ref& s) {
  return (DEREF(s).getClassType() == AR_ALLOCATE_STATEMENT_CLASS_TYPE);
}

inline bool is_load_stmt(const Statement_ref& s) {
  return (DEREF(s).getClassType() == AR_LOAD_STATEMENT_CLASS_TYPE);
}

inline bool is_store_stmt(const Statement_ref& s) {
  return (DEREF(s).getClassType() == AR_STORE_STATEMENT_CLASS_TYPE);
}

inline bool is_pointer_shift_stmt(const Statement_ref& s) {
  return (DEREF(s).getClassType() == AR_POINTER_SHIFT_STATEMENT_CLASS_TYPE);
}

inline bool is_memcpy_stmt(const Statement_ref& s) {
  return (DEREF(s).getClassType() == AR_MEMCPY_STATEMENT_CLASS_TYPE);
}

inline bool is_memmove_stmt(const Statement_ref& s) {
  return (DEREF(s).getClassType() == AR_MEMMOVE_STATEMENT_CLASS_TYPE);
}

inline bool is_memset_stmt(const Statement_ref& s) {
  return (DEREF(s).getClassType() == AR_MEMSET_STATEMENT_CLASS_TYPE);
}

inline bool is_nop_stmt(const Statement_ref& s) {
  return (DEREF(s).getClassType() == AR_NOP_STATEMENT_CLASS_TYPE);
}

inline bool is_unreachable_stmt(const Statement_ref& s) {
  return (DEREF(s).getClassType() == AR_UNREACHABLE_STATEMENT_CLASS_TYPE);
}

inline bool is_invoke_stmt(const Statement_ref& stmt) {
  return (DEREF(stmt).getClassType() == AR_INVOKE_STATEMENT_CLASS_TYPE);
}

inline bool is_resume_stmt(const Statement_ref& stmt) {
  return (DEREF(stmt).getClassType() == AR_RESUME_STATEMENT_CLASS_TYPE);
}

} // end namespace ar

} // end namespace arbos

namespace arbos {

// Customized hasher for Operand_ref
struct Operand_Hasher {
  static uint64_t _getUID(Operand_ref o) {
    if (ar::ar_internal::is_var_operand(o)) {
      Var_Operand_ref v = node_cast< Var_Operand >(o);
      Internal_Variable_ref i_v = DEREF(v).getInternalVariable();
      return ar::getUID(i_v);
    } else {
      assert(ar::ar_internal::is_cst_operand(o));
      Cst_Operand_ref cst_operand = node_cast< Cst_Operand >(o);
      Constant_ref cst = DEREF(cst_operand).getConstant();
      return ar::getUID(cst);
    }
  }

  struct eq {
    bool operator()(Operand_ref a, Operand_ref b) const {
      return _getUID(a) == _getUID(b);
    }
  };
  struct hash {
    unsigned int operator()(Operand_ref o) const { return _getUID(o); }
  };
};

// Customized hasher for Call_ref
struct Call_Hasher {
  struct eq {
    bool operator()(Call_ref a, Call_ref b) const {
      return ar::getUID(a) == ar::getUID(b);
    }
  };
  struct hash {
    unsigned int operator()(Call_ref a) const {
      return ar::getUID(a);
    }
  };
};

// Customized hasher for Global_Variable_ref
struct Global_Variable_Hasher {
  struct eq {
    bool operator()(Global_Variable_ref a, Global_Variable_ref b) const {
      return ar::getUID(a) == ar::getUID(b);
    }
  };
  struct hash {
    unsigned int operator()(Global_Variable_ref a) const {
      return ar::getUID(a);
    }
  };
};

// Customized hasher for Internal_Variable_ref
struct Internal_Variable_Hasher {
  struct eq {
    bool operator()(Internal_Variable_ref a, Internal_Variable_ref b) const {
      return ar::getUID(a) == ar::getUID(b);
    }
  };
  struct hash {
    unsigned int operator()(Internal_Variable_ref a) const {
      return ar::getUID(a);
    }
  };
};

// Customized hasher for Local_Variable_ref
struct Local_Variable_Hasher {
  struct eq {
    bool operator()(Local_Variable_ref a, Local_Variable_ref b) const {
      return ar::getUID(a) == ar::getUID(b);
    }
  };
  struct hash {
    unsigned int operator()(Local_Variable_ref a) const {
      return ar::getUID(a);
    }
  };
};

std::size_t hash_value(Call_ref v) {
  return Call_Hasher::hash()(v);
}
std::size_t hash_value(Internal_Variable_ref v) {
  return Internal_Variable_Hasher::hash()(v);
}
std::size_t hash_value(Operand_ref o) {
  return Operand_Hasher::hash()(o);
}
std::size_t hash_value(Local_Variable_ref v) {
  return Local_Variable_Hasher::hash()(v);
}
std::size_t hash_value(Var_Addr_Constant_ref v) {
  return ar::getUID(v);
}
std::size_t hash_value(Basic_Block_ref b) {
  return ar::getUID(b);
}
std::size_t hash_value(Function_ref f) {
  return ar::getUID(f);
}

} // end namespace arbos

namespace std {

template <>
struct hash< arbos::Call_ref > {
  std::size_t operator()(const arbos::Call_ref& v) const {
    return arbos::Call_Hasher::hash()(v);
  }
};

template <>
struct hash< arbos::Internal_Variable_ref > {
  std::size_t operator()(const arbos::Internal_Variable_ref& v) const {
    return arbos::Internal_Variable_Hasher::hash()(v);
  }
};

template <>
struct hash< arbos::Operand_ref > {
  std::size_t operator()(const arbos::Operand_ref& o) const {
    return arbos::Operand_Hasher::hash()(o);
  }
};

template <>
struct hash< arbos::Global_Variable_ref > {
  std::size_t operator()(const arbos::Global_Variable_ref& v) const {
    return arbos::Global_Variable_Hasher::hash()(v);
  }
};

template <>
struct hash< arbos::Local_Variable_ref > {
  std::size_t operator()(const arbos::Local_Variable_ref& v) const {
    return arbos::Local_Variable_Hasher::hash()(v);
  }
};

template <>
struct hash< arbos::Basic_Block_ref > {
  std::size_t operator()(const arbos::Basic_Block_ref& b) const {
    return arbos::ar::getUID(b);
  }
};

template <>
struct hash< arbos::Function_ref > {
  std::size_t operator()(const arbos::Function_ref& f) const {
    return arbos::ar::getUID(f);
  }
};

} // end namespace std

namespace arbos {

template < typename T1, typename T2 >
inline bool IsEqual(T1 x, T2 y) {
  return false;
}

template <>
inline bool IsEqual(Internal_Variable_ref x, Internal_Variable_ref y) {
  Internal_Variable_Hasher::hash h;
  return h(x) == h(y);
}

template <>
inline bool IsEqual(Operand_ref x, Operand_ref y) {
  Operand_Hasher::hash h;
  return h(x) == h(y);
}

template <>
inline bool IsEqual(Internal_Variable_ref x, Operand_ref y) {
  Internal_Variable_Hasher::hash h1;
  Operand_Hasher::hash h2;
  return h1(x) == h2(y);
}

template <>
inline bool IsEqual(Operand_ref x, Internal_Variable_ref y) {
  Operand_Hasher::hash h1;
  Internal_Variable_Hasher::hash h2;
  return h1(x) == h2(y);
}

template < typename T1, typename T2 >
struct is_same_type {
  static const bool value = false;
};

template <>
struct is_same_type< Internal_Variable_ref, Internal_Variable_ref > {
  static const bool value = true;
};

template <>
struct is_same_type< Operand_ref, Operand_ref > {
  static const bool value = true;
};

template < typename T1, typename T2 >
void convert(T1 x, T2& y);

template <>
inline void convert(Internal_Variable_ref x, Internal_Variable_ref& y) {
  y = x;
}

template <>
inline void convert(Operand_ref x, Operand_ref& y) {
  y = x;
}

template <>
inline void convert(Internal_Variable_ref x, Operand_ref& y) {
  y = AR_Var_Operand::create(x);
}

template <>
inline void convert(Constant_ref x, Operand_ref& y) {
  y = AR_Cst_Operand::create(x);
}

template <>
inline void convert(Var_Addr_Constant_ref x, Operand_ref& y) {
  y = AR_Cst_Operand::create(x);
}

} // end namespace arbos

namespace arbos {
/////
//! printing of operand and statements
/////

std::ostream& operator<<(std::ostream& o, const Operand_ref& op) {
  if (ar::ar_internal::is_var_operand(op)) {
    Var_Operand_ref v = node_cast< Var_Operand >(op);
    Internal_Variable_ref i_v = DEREF(v).getInternalVariable();
    o << DEREF(i_v).getName();
  } else {
    assert(ar::ar_internal::is_cst_operand(op));

    Cst_Operand_ref cst_operand = node_cast< Cst_Operand >(op);
    Constant_ref cst = DEREF(cst_operand).getConstant();
    if (ar::ar_internal::is_int_constant(cst)) {
      Integer_Constant_ref n = node_cast< Integer_Constant >(cst);
      o << DEREF(n).getValue();
    } else if (ar::ar_internal::is_float_constant(cst)) {
      FP_Constant_ref n = node_cast< FP_Constant >(cst);
      fp_number fp_n = DEREF(n).getValue();
      o << fp_n;
    } else if (ar::ar_internal::is_null_constant(cst)) {
      o << "$null";
    } else if (ar::ar_internal::is_var_addr_constant(cst)) {
      Var_Addr_Constant_ref var_cst = node_cast< Var_Addr_Constant >(cst);
      o << DEREF(DEREF(var_cst).getVariable()).getNameId();
    } else if (ar::ar_internal::is_function_addr_constant(cst)) {
      Function_Addr_Constant_ref faddr =
          node_cast< Function_Addr_Constant >(cst);
      DEREF(faddr).print(o);
    } else if (ar::ar_internal::is_undefined_constant(cst)) {
      o << "$undefined";
    } else {
      o << "$unknown_cst";
    }
  }
  return o;
}
std::ostream& operator<<(std::ostream& o, const Type_ref& r) {
  DEREF(r).print(o);
  return o;
}
std::ostream& operator<<(std::ostream& o, const Function_ref& r) {
  o << ar::getName(r);
  return o;
}
std::ostream& operator<<(std::ostream& o, const Basic_Block_ref& r) {
  o << ar::getName(r);
  return o;
}
std::ostream& operator<<(std::ostream& o, const Arith_Op_ref& r) {
  DEREF(r).print(o);
  return o;
}
std::ostream& operator<<(std::ostream& o, const Integer_Comparison_ref& r) {
  DEREF(r).print(o);
  return o;
}
std::ostream& operator<<(std::ostream& o, const FP_Op_ref& r) {
  DEREF(r).print(o);
  return o;
}
std::ostream& operator<<(std::ostream& o, const FP_Comparison_ref& r) {
  DEREF(r).print(o);
  return o;
}
std::ostream& operator<<(std::ostream& o, const Assignment_ref& r) {
  DEREF(r).print(o);
  return o;
}
std::ostream& operator<<(std::ostream& o, const Conv_Op_ref& r) {
  DEREF(r).print(o);
  return o;
}
std::ostream& operator<<(std::ostream& o, const Bitwise_Op_ref& r) {
  DEREF(r).print(o);
  return o;
}
std::ostream& operator<<(std::ostream& o, const Allocate_ref& r) {
  DEREF(r).print(o);
  return o;
}
std::ostream& operator<<(std::ostream& o, const Store_ref& r) {
  DEREF(r).print(o);
  return o;
}
std::ostream& operator<<(std::ostream& o, const Load_ref& r) {
  DEREF(r).print(o);
  return o;
}
std::ostream& operator<<(std::ostream& o, const Pointer_Shift_ref& r) {
  DEREF(r).print(o);
  return o;
}
std::ostream& operator<<(std::ostream& o, const MemCpy_ref& r) {
  DEREF(r).print(o);
  return o;
}
std::ostream& operator<<(std::ostream& o, const MemMove_ref& r) {
  DEREF(r).print(o);
  return o;
}
std::ostream& operator<<(std::ostream& o, const MemSet_ref& r) {
  DEREF(r).print(o);
  return o;
}
std::ostream& operator<<(std::ostream& o, const Call_ref& r) {
  DEREF(r).print(o);
  return o;
}
std::ostream& operator<<(std::ostream& o, const Invoke_ref& r) {
  DEREF(r).print(o);
  return o;
}
std::ostream& operator<<(std::ostream& o, const Return_Value_ref& r) {
  DEREF(r).print(o);
  return o;
}
std::ostream& operator<<(std::ostream& o, const Landing_Pad_ref& r) {
  DEREF(r).print(o);
  return o;
}
std::ostream& operator<<(std::ostream& o, const Resume_ref& r) {
  DEREF(r).print(o);
  return o;
}

} // end namespace arbos

#endif // ANALYZER_WRAPPER_HPP
