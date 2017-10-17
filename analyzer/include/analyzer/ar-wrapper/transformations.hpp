/*******************************************************************************
 *
 * Common classes and types for performing ARBOS IR transformations
 *
 * Authors: Jorge A. Navas
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

#ifndef ANALYZER_TRANSFORMATIONS_HPP
#define ANALYZER_TRANSFORMATIONS_HPP

#include <boost/optional.hpp>

#include <analyzer/ar-wrapper/wrapper.hpp>

namespace arbos {

namespace transformations {

/*
   Begin interface to perform in-place modifications of ARBOS AR.
*/

//! In-place modification of binary statements
template < typename Binary_Statement_ref >
void inline setResult(Binary_Statement_ref s, Internal_Variable_ref v) {
  DEREF(s).setResult(v);
}
template < typename Binary_Statement_ref >
void inline setLeftOp(Binary_Statement_ref s, Operand_ref o) {
  DEREF(s).setLeftOperand(o);
}
template < typename Binary_Statement_ref >
void inline setRightOp(Binary_Statement_ref s, Operand_ref o) {
  DEREF(s).setRightOperand(o);
}

//! In-place modification of assignments
void inline setLeftOp(Assignment_ref s, Internal_Variable_ref v) {
  DEREF(s).setLeftOperand(v);
}
void inline setRightOp(Assignment_ref s, Operand_ref o) {
  DEREF(s).setRightOperand(o);
}

//! In-place modification of allocate
void inline setArraySize(Allocate_ref s, Operand_ref o) {
  DEREF(s).setArraySize(o);
}

//! In-place modification of loads
void inline setResult(Load_ref s, Internal_Variable_ref v) {
  DEREF(s).setResult(v);
}
void inline setPointer(Load_ref s, Operand_ref p) {
  DEREF(s).setPointer(p);
}

//! In-place modification of stores
void inline setValue(Store_ref s, Operand_ref p) {
  DEREF(s).setValue(p);
}
void inline setPointer(Store_ref s, Operand_ref p) {
  DEREF(s).setPointer(p);
}

//! In-place modification of extractelem
void inline setResult(Extract_Element_ref s, Internal_Variable_ref v) {
  DEREF(s).setResult(v);
}
void inline setAggregate(Extract_Element_ref s, Operand_ref v) {
  DEREF(s).setValue(v);
}
void inline setOffset(Extract_Element_ref s, Operand_ref o) {
  DEREF(s).setOffset(o);
}

//! In-place modification of insertelem
void inline setResult(Insert_Element_ref s, Internal_Variable_ref v) {
  DEREF(s).setResult(v);
}
void inline setAggregate(Insert_Element_ref s, Operand_ref v) {
  DEREF(s).setValue(v);
}
void inline setOffset(Insert_Element_ref s, Operand_ref o) {
  DEREF(s).setOffset(o);
}
void inline setElement(Insert_Element_ref s, Operand_ref e) {
  DEREF(s).setElement(e);
}

//! In-place modification of pointer arithmetic calculations
void inline setResult(Pointer_Shift_ref s, Internal_Variable_ref v) {
  DEREF(s).setResult(v);
}
void inline setPointer(Pointer_Shift_ref s, Operand_ref p) {
  DEREF(s).setPointer(p);
}
void inline setOffset(Pointer_Shift_ref s, Operand_ref o) {
  DEREF(s).setOffset(o);
}

//! In-place modification of abstracted variable
void inline setVar(Abstract_Variable_ref s, Internal_Variable_ref v) {
  DEREF(s).setVariable(v);
}

//! In-place modification of landing pad
void inline setVar(Landing_Pad_ref s, Internal_Variable_ref v) {
  DEREF(s).setException(v);
}

//! In-place modification of resume
void inline setVar(Resume_ref s, Internal_Variable_ref v) {
  DEREF(s).setException(v);
}

//! In-place modification of abstracted memory locations
void inline setPointer(Abstract_Memory_ref s, Operand_ref p) {
  DEREF(s).set_pointer(p);
}
void inline setLen(Abstract_Memory_ref s, Operand_ref o) {
  DEREF(s).set_length(o);
}

//! In-place modification of mempcy
void inline setSource(MemCpy_ref s, Operand_ref p) {
  DEREF(s).setSource(p);
}
void inline setTarget(MemCpy_ref s, Operand_ref p) {
  DEREF(s).setTarget(p);
}
void inline setLen(MemCpy_ref s, Operand_ref o) {
  DEREF(s).setLength(o);
}

//! In-place modification of memmove
void inline setSource(MemMove_ref s, Operand_ref p) {
  DEREF(s).setSource(p);
}
void inline setTarget(MemMove_ref s, Operand_ref p) {
  DEREF(s).setTarget(p);
}
void inline setLen(MemMove_ref s, Operand_ref o) {
  DEREF(s).setLength(o);
}

//! In-place modification of memset
void inline setBase(MemSet_ref s, Operand_ref p) {
  DEREF(s).setOperand(p);
}
void inline setLen(MemSet_ref s, Operand_ref o) {
  DEREF(s).setLength(o);
}
void inline setValue(MemSet_ref s, Operand_ref o) {
  DEREF(s).setValue(o);
}

//! In-place modification of integer comparisons
void inline setLeftOp(Integer_Comparison_ref s, Operand_ref o) {
  DEREF(s).setLeftOperand(o);
}
void inline setRightOp(Integer_Comparison_ref s, Operand_ref o) {
  DEREF(s).setRightOperand(o);
}

//! In-place modification of fp comparisons
void inline setLeftOp(FP_Comparison_ref s, Operand_ref o) {
  DEREF(s).setLeftOperand(o);
}
void inline setRightOp(FP_Comparison_ref s, Operand_ref o) {
  DEREF(s).setRightOperand(o);
}

//! In-place modification of casting
void inline setLeftOp(Conv_Op_ref s, Operand_ref o) {
  DEREF(s).setResult(o);
}
void inline setRightOp(Conv_Op_ref s, Operand_ref o) {
  DEREF(s).setOperand(o);
}

//! In-place modification of return values of functions
void inline setReturnValue(Return_Value_ref s, Operand_ref o) {
  DEREF(s).setValue(o);
}

//! In-place modification of the actual parameters of a call site
void inline setActualParams(Call_ref cs, OpRange args) {
  DEREF(cs).setArguments(args);
}
//! In-place modification of the return value of the call site
void inline setReturnValue(Call_ref cs, Internal_Variable_ref r) {
  DEREF(cs).setReturnValue(r);
}

//! In-place modification of va_start
void inline setPointer(VA_Start_ref s, Operand_ref p) {
  DEREF(s).setPointer(p);
}

//! In-place modification of va_end
void inline setPointer(VA_End_ref s, Operand_ref p) {
  DEREF(s).setPointer(p);
}

//! In-place modification of va_arg
void inline setPointer(VA_Arg_ref s, Operand_ref p) {
  DEREF(s).setPointer(p);
}

//! In-place modification of va_copy
void inline setSource(VA_Copy_ref s, Operand_ref p) {
  DEREF(s).setSource(p);
}
void inline setDestination(VA_Copy_ref s, Operand_ref p) {
  DEREF(s).setDestination(p);
}

//! return the value of op is it is an integer constant
inline boost::optional< z_number > getIntCst(Operand_ref op) {
  if (!ar::ar_internal::is_cst_operand(op))
    return boost::optional< z_number >();

  Cst_Operand_ref cst_operand = node_cast< Cst_Operand >(op);
  Constant_ref cst = ar::getConstant(cst_operand);

  if (!ar::ar_internal::is_int_constant(cst))
    return boost::optional< z_number >();

  Integer_Constant_ref n = node_cast< Integer_Constant >(cst);
  return boost::optional< z_number >(ar::getZValue(n));
}

inline Operand_ref makeIntCst(z_number n, Type_ref ty) {
  assert(ar::ar_internal::is_integer_type(ty));
  Integer_Type_ref int_ty = node_cast< Integer_Type >(ty);
  return AR_Cst_Operand::create(AR_Integer_Constant::create(int_ty, n));
}

//! insert a new statement at the beginning of the block
inline void addFrontStatement(Basic_Block_ref b, Statement_ref s) {
  DEREF(b).insertFront(s);
}
//! insert a new statement at the end of the block
inline void addBackStatement(Basic_Block_ref b, Statement_ref s) {
  DEREF(b).pushBack(s);
}
//! remove a statement from a basic block
inline void removeStatement(Basic_Block_ref b, Statement_ref s) {
  DEREF(b).remove(s);
}
//! replace a statement in a basic block
inline void replaceStatement(Basic_Block_ref b,
                             Statement_ref old_s,
                             Statement_ref new_s) {
  DEREF(b).replace(old_s, new_s);
}

/*
   End interface
*/

// Statement visitor to replace operands
template < typename T >
class stmt_replace_operands : public arbos_visitor_api {
public:
  typedef std::shared_ptr< stmt_replace_operands > stmt_replace_ptr;

  typedef std::pair< Operand_ref, T > substitution_t;

private:
  substitution_t _sigma;
  bool _has_changed;

public:
  stmt_replace_operands(substitution_t sigma)
      : arbos_visitor_api(), _sigma(sigma), _has_changed(false) {}

  bool has_changed() const { return _has_changed; }

  void visit(Arith_Op_ref stmt) {
    if (IsEqual< Operand_ref, Operand_ref >(ar::getLeftOp(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getLeftOp(stmt), op)) {
        setLeftOp(stmt, op);
        _has_changed = true;
      }
    }
    if (IsEqual< Operand_ref, Operand_ref >(ar::getRightOp(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getRightOp(stmt), op)) {
        setRightOp(stmt, op);
        _has_changed = true;
      }
    }
  }

  void visit(FP_Op_ref stmt) {
    if (IsEqual< Operand_ref, Operand_ref >(ar::getLeftOp(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getLeftOp(stmt), op)) {
        setLeftOp(stmt, op);
        _has_changed = true;
      }
    }

    if (IsEqual< Operand_ref, Operand_ref >(ar::getRightOp(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getRightOp(stmt), op)) {
        setRightOp(stmt, op);
        _has_changed = true;
      }
    }
  }

  void visit(Assignment_ref stmt) {
    if (IsEqual< Operand_ref, Operand_ref >(ar::getRightOp(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getRightOp(stmt), op)) {
        setRightOp(stmt, op);
        _has_changed = true;
      }
    }
  }

  void visit(Bitwise_Op_ref stmt) {
    if (IsEqual< Operand_ref, Operand_ref >(ar::getLeftOp(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getLeftOp(stmt), op)) {
        setLeftOp(stmt, op);
        _has_changed = true;
      }
    }

    if (IsEqual< Operand_ref, Operand_ref >(ar::getRightOp(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getRightOp(stmt), op)) {
        setRightOp(stmt, op);
        _has_changed = true;
      }
    }
  }

  void visit(Pointer_Shift_ref stmt) {
    if (IsEqual< Operand_ref, Operand_ref >(ar::getBase(stmt), _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getBase(stmt), op)) {
        setPointer(stmt, op);
        _has_changed = true;
      }
    }

    if (IsEqual< Operand_ref, Operand_ref >(ar::getOffset(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getOffset(stmt), op)) {
        setOffset(stmt, op);
        _has_changed = true;
      }
    }
  }

  void visit(Integer_Comparison_ref stmt) {
    if (IsEqual< Operand_ref, Operand_ref >(ar::getLeftOp(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getLeftOp(stmt), op)) {
        setLeftOp(stmt, op);
        _has_changed = true;
      }
    }

    if (IsEqual< Operand_ref, Operand_ref >(ar::getRightOp(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getRightOp(stmt), op)) {
        setRightOp(stmt, op);
        _has_changed = true;
      }
    }
  }

  void visit(FP_Comparison_ref stmt) {
    if (IsEqual< Operand_ref, Operand_ref >(ar::getLeftOp(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getLeftOp(stmt), op)) {
        setLeftOp(stmt, op);
        _has_changed = true;
      }
    }

    if (IsEqual< Operand_ref, Operand_ref >(ar::getRightOp(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getRightOp(stmt), op)) {
        setRightOp(stmt, op);
        _has_changed = true;
      }
    }
  }

  void visit(Conv_Op_ref stmt) {
    if (IsEqual< Operand_ref, Operand_ref >(ar::getRightOp(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getRightOp(stmt), op)) {
        setRightOp(stmt, op);
        _has_changed = true;
      }
    }

    if (IsEqual< Operand_ref, Operand_ref >(ar::getLeftOp(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getLeftOp(stmt), op)) {
        setLeftOp(stmt, op);
        _has_changed = true;
      }
    }
  }

  void visit(Allocate_ref stmt) {
    if (IsEqual< Operand_ref, Operand_ref >(ar::getArraySize(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getArraySize(stmt), op)) {
        setArraySize(stmt, op);
        _has_changed = true;
      }
    }
  }

  void visit(Load_ref stmt) {
    if (IsEqual< Operand_ref, Operand_ref >(ar::getPointer(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getPointer(stmt), op)) {
        setPointer(stmt, op);
        _has_changed = true;
      }
    }
  }

  void visit(Store_ref stmt) {
    if (IsEqual< Operand_ref, Operand_ref >(ar::getValue(stmt), _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getValue(stmt), op)) {
        setValue(stmt, op);
        _has_changed = true;
      }
    }

    if (IsEqual< Operand_ref, Operand_ref >(ar::getPointer(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getPointer(stmt), op)) {
        setPointer(stmt, op);
        _has_changed = true;
      }
    }
  }

  void visit(Extract_Element_ref stmt) {
    if (IsEqual< Operand_ref, Operand_ref >(ar::getAggregate(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getAggregate(stmt), op)) {
        setAggregate(stmt, op);
        _has_changed = true;
      }
    }

    if (IsEqual< Operand_ref, Operand_ref >(ar::getOffset(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getOffset(stmt), op)) {
        setOffset(stmt, op);
        _has_changed = true;
      }
    }
  }

  void visit(Insert_Element_ref stmt) {
    if (IsEqual< Operand_ref, Operand_ref >(ar::getAggregate(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getAggregate(stmt), op)) {
        setAggregate(stmt, op);
        _has_changed = true;
      }
    }

    if (IsEqual< Operand_ref, Operand_ref >(ar::getOffset(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getOffset(stmt), op)) {
        setOffset(stmt, op);
        _has_changed = true;
      }
    }

    if (IsEqual< Operand_ref, Operand_ref >(ar::getElement(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getElement(stmt), op)) {
        setElement(stmt, op);
        _has_changed = true;
      }
    }
  }

  void visit(Abstract_Memory_ref stmt) {
    if (IsEqual< Operand_ref, Operand_ref >(ar::getPointer(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getPointer(stmt), op)) {
        setPointer(stmt, op);
        _has_changed = true;
      }
    }

    if (IsEqual< Operand_ref, Operand_ref >(ar::getLen(stmt), _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getLen(stmt), op)) {
        setLen(stmt, op);
        _has_changed = true;
      }
    }
  }

  void visit(Return_Value_ref stmt) {
    if (!ar::getReturnValue(stmt))
      return;

    if (IsEqual< Operand_ref, Operand_ref >(*ar::getReturnValue(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(*ar::getReturnValue(stmt), op)) {
        setReturnValue(stmt, op);
        _has_changed = true;
      }
    }
  }

  void visit(MemCpy_ref stmt) {
    if (IsEqual< Operand_ref, Operand_ref >(ar::getTarget(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getTarget(stmt), op)) {
        setTarget(stmt, op);
        _has_changed = true;
      }
    }

    if (IsEqual< Operand_ref, Operand_ref >(ar::getSource(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getSource(stmt), op)) {
        setSource(stmt, op);
        _has_changed = true;
      }
    }

    if (IsEqual< Operand_ref, Operand_ref >(ar::getLen(stmt), _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getLen(stmt), op)) {
        setLen(stmt, op);
        _has_changed = true;
      }
    }
  }

  void visit(MemMove_ref stmt) {
    if (IsEqual< Operand_ref, Operand_ref >(ar::getTarget(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getTarget(stmt), op)) {
        setTarget(stmt, op);
        _has_changed = true;
      }
    }

    if (IsEqual< Operand_ref, Operand_ref >(ar::getSource(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getSource(stmt), op)) {
        setSource(stmt, op);
        _has_changed = true;
      }
    }

    if (IsEqual< Operand_ref, Operand_ref >(ar::getLen(stmt), _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getLen(stmt), op)) {
        setLen(stmt, op);
        _has_changed = true;
      }
    }
  }

  void visit(MemSet_ref stmt) {
    if (IsEqual< Operand_ref, Operand_ref >(ar::getLen(stmt), _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getLen(stmt), op)) {
        setLen(stmt, op);
        _has_changed = true;
      }
    }

    if (IsEqual< Operand_ref, Operand_ref >(ar::getBase(stmt), _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getBase(stmt), op)) {
        setBase(stmt, op);
        _has_changed = true;
      }
    }

    if (IsEqual< Operand_ref, Operand_ref >(ar::getValue(stmt), _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getValue(stmt), op)) {
        setValue(stmt, op);
        _has_changed = true;
      }
    }
  }

  void visit(Call_ref stmt) {
    OpRange args = ar::getArguments(stmt);
    OpRange new_args;
    for (OpRange::iterator it = args.begin(); it != args.end(); ++it) {
      if (IsEqual< Operand_ref, Operand_ref >(*it, _sigma.first)) {
        Operand_ref op = Null_ref;
        convert< T, Operand_ref >(_sigma.second, op);
        new_args.push_back(op);
        if (!IsEqual< Operand_ref, Operand_ref >(*it, op))
          _has_changed = true;
      } else
        new_args.push_back(*it);
    }
    setActualParams(stmt, new_args);
  }

  void visit(Invoke_ref stmt) { visit(ar::getFunctionCall(stmt)); }

  void visit(VA_Start_ref stmt) {
    if (IsEqual< Operand_ref, Operand_ref >(ar::getPointer(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getPointer(stmt), op)) {
        setPointer(stmt, op);
        _has_changed = true;
      }
    }
  }

  void visit(VA_End_ref stmt) {
    if (IsEqual< Operand_ref, Operand_ref >(ar::getPointer(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getPointer(stmt), op)) {
        setPointer(stmt, op);
        _has_changed = true;
      }
    }
  }

  void visit(VA_Arg_ref stmt) {
    if (IsEqual< Operand_ref, Operand_ref >(ar::getPointer(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getPointer(stmt), op)) {
        setPointer(stmt, op);
        _has_changed = true;
      }
    }
  }

  void visit(VA_Copy_ref stmt) {
    if (IsEqual< Operand_ref, Operand_ref >(ar::getSource(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getSource(stmt), op)) {
        setSource(stmt, op);
        _has_changed = true;
      }
    }

    if (IsEqual< Operand_ref, Operand_ref >(ar::getDestination(stmt),
                                            _sigma.first)) {
      Operand_ref op = Null_ref;
      convert< T, Operand_ref >(_sigma.second, op);
      if (!IsEqual< Operand_ref, Operand_ref >(ar::getDestination(stmt), op)) {
        setDestination(stmt, op);
        _has_changed = true;
      }
    }
  }

  // NOT IMPLEMENTED
  void visit(Abstract_Variable_ref) {}
  void visit(Landing_Pad_ref) {}
  void visit(Resume_ref) {}
  void visit(Unreachable_ref) {}

}; // end class stmt_replace_operands

//! Statement visitor to replace internal variables
template < typename T >
class stmt_replace_internal_vars : public arbos_visitor_api {
public:
  typedef std::shared_ptr< stmt_replace_internal_vars > stmt_replace_ptr;
  typedef std::pair< Internal_Variable_ref, T > substitution_t;

private:
  substitution_t _sigma;
  bool _has_changed;

public:
  stmt_replace_internal_vars(substitution_t sigma)
      : arbos_visitor_api(), _sigma(sigma), _has_changed(false) {}

  bool has_changed() const { return _has_changed; }

  void visit(Arith_Op_ref stmt) {
    if (IsEqual< Internal_Variable_ref, Internal_Variable_ref >(ar::getResult(
                                                                    stmt),
                                                                _sigma.first)) {
      assert((is_same_type< Internal_Variable_ref, T >::value) &&
             "type mismatch during transformation");

      if (!IsEqual< Internal_Variable_ref,
                    Internal_Variable_ref >(ar::getResult(stmt),
                                            _sigma.second)) {
        setResult(stmt, _sigma.second);
        _has_changed = true;
      }
    }
  }

  void visit(FP_Op_ref stmt) {
    if (IsEqual< Internal_Variable_ref, Internal_Variable_ref >(ar::getResult(
                                                                    stmt),
                                                                _sigma.first)) {
      assert((is_same_type< Internal_Variable_ref, T >::value) &&
             "type mismatch during transformation");

      if (!IsEqual< Internal_Variable_ref,
                    Internal_Variable_ref >(ar::getResult(stmt),
                                            _sigma.second)) {
        setResult(stmt, _sigma.second);
        _has_changed = true;
      }
    }
  }

  void visit(Assignment_ref stmt) {
    if (IsEqual< Internal_Variable_ref, Internal_Variable_ref >(ar::getLeftOp(
                                                                    stmt),
                                                                _sigma.first)) {
      assert((is_same_type< Internal_Variable_ref, T >::value) &&
             "type mismatch during transformation");

      if (!IsEqual< Internal_Variable_ref,
                    Internal_Variable_ref >(ar::getLeftOp(stmt),
                                            _sigma.second)) {
        setLeftOp(stmt, _sigma.second);
        _has_changed = true;
      }
    }
  }

  void visit(Bitwise_Op_ref stmt) {
    if (IsEqual< Internal_Variable_ref, Internal_Variable_ref >(ar::getResult(
                                                                    stmt),
                                                                _sigma.first)) {
      assert((is_same_type< Internal_Variable_ref, T >::value) &&
             "type mismatch during transformation");

      if (!IsEqual< Internal_Variable_ref,
                    Internal_Variable_ref >(ar::getResult(stmt),
                                            _sigma.second)) {
        setResult(stmt, _sigma.second);
        _has_changed = true;
      }
    }
  }

  void visit(Load_ref stmt) {
    if (IsEqual< Internal_Variable_ref, Internal_Variable_ref >(ar::getResult(
                                                                    stmt),
                                                                _sigma.first)) {
      assert((is_same_type< Internal_Variable_ref, T >::value) &&
             "type mismatch during transformation");
      if (!IsEqual< Internal_Variable_ref,
                    Internal_Variable_ref >(ar::getResult(stmt),
                                            _sigma.second)) {
        setResult(stmt, _sigma.second);
        _has_changed = true;
      }
    }
  }

  void visit(Extract_Element_ref stmt) {
    if (IsEqual< Internal_Variable_ref, Internal_Variable_ref >(ar::getResult(
                                                                    stmt),
                                                                _sigma.first)) {
      assert((is_same_type< Internal_Variable_ref, T >::value) &&
             "type mismatch during transformation");
      if (!IsEqual< Internal_Variable_ref,
                    Internal_Variable_ref >(ar::getResult(stmt),
                                            _sigma.second)) {
        setResult(stmt, _sigma.second);
        _has_changed = true;
      }
    }
  }

  void visit(Insert_Element_ref stmt) {
    if (IsEqual< Internal_Variable_ref, Internal_Variable_ref >(ar::getResult(
                                                                    stmt),
                                                                _sigma.first)) {
      assert((is_same_type< Internal_Variable_ref, T >::value) &&
             "type mismatch during transformation");
      if (!IsEqual< Internal_Variable_ref,
                    Internal_Variable_ref >(ar::getResult(stmt),
                                            _sigma.second)) {
        setResult(stmt, _sigma.second);
        _has_changed = true;
      }
    }
  }

  void visit(Pointer_Shift_ref stmt) {
    if (IsEqual< Internal_Variable_ref, Internal_Variable_ref >(ar::getResult(
                                                                    stmt),
                                                                _sigma.first)) {
      assert((is_same_type< Internal_Variable_ref, T >::value) &&
             "type mismatch during transformation");
      if (!IsEqual< Internal_Variable_ref,
                    Internal_Variable_ref >(ar::getResult(stmt),
                                            _sigma.second)) {
        setResult(stmt, _sigma.second);
        _has_changed = true;
      }
    }
  }

  void visit(Abstract_Variable_ref stmt) {
    if (IsEqual< Internal_Variable_ref, Internal_Variable_ref >(ar::getVar(
                                                                    stmt),
                                                                _sigma.first)) {
      assert((is_same_type< Internal_Variable_ref, T >::value) &&
             "type mismatch during transformation");
      if (!IsEqual< Internal_Variable_ref,
                    Internal_Variable_ref >(ar::getVar(stmt), _sigma.second)) {
        setVar(stmt, _sigma.second);
        _has_changed = true;
      }
    }
  }

  void visit(Call_ref stmt) {
    if (ar::getReturnValue(stmt)) {
      if (IsEqual< Internal_Variable_ref,
                   Internal_Variable_ref >(*(ar::getReturnValue(stmt)),
                                           _sigma.first)) {
        assert((is_same_type< Internal_Variable_ref, T >::value) &&
               "type mismatch during transformation");
        if (!IsEqual< Internal_Variable_ref,
                      Internal_Variable_ref >(*(ar::getReturnValue(stmt)),
                                              _sigma.second)) {
          setReturnValue(stmt, _sigma.second);
          _has_changed = true;
        }
      }
    }
  }

  void visit(Invoke_ref stmt) { visit(ar::getFunctionCall(stmt)); }

  void visit(Landing_Pad_ref stmt) {
    if (IsEqual< Internal_Variable_ref, Internal_Variable_ref >(ar::getVar(
                                                                    stmt),
                                                                _sigma.first)) {
      assert((is_same_type< Internal_Variable_ref, T >::value) &&
             "type mismatch during transformation");
      if (!IsEqual< Internal_Variable_ref,
                    Internal_Variable_ref >(ar::getVar(stmt), _sigma.second)) {
        setVar(stmt, _sigma.second);
        _has_changed = true;
      }
    }
  }

  void visit(Resume_ref stmt) {
    if (IsEqual< Internal_Variable_ref, Internal_Variable_ref >(ar::getVar(
                                                                    stmt),
                                                                _sigma.first)) {
      assert((is_same_type< Internal_Variable_ref, T >::value) &&
             "type mismatch during transformation");
      if (!IsEqual< Internal_Variable_ref,
                    Internal_Variable_ref >(ar::getVar(stmt), _sigma.second)) {
        setVar(stmt, _sigma.second);
        _has_changed = true;
      }
    }
  }

  void visit(VA_Arg_ref stmt) {
    if (IsEqual< Internal_Variable_ref, Internal_Variable_ref >(ar::getResult(
                                                                    stmt),
                                                                _sigma.first)) {
      assert((is_same_type< Internal_Variable_ref, T >::value) &&
             "type mismatch during transformation");
      if (!IsEqual< Internal_Variable_ref,
                    Internal_Variable_ref >(ar::getResult(stmt),
                                            _sigma.second)) {
        setResult(stmt, _sigma.second);
        _has_changed = true;
      }
    }
  }

  // NOT IMPLEMENTED
  void visit(Integer_Comparison_ref) {}
  void visit(FP_Comparison_ref) {}
  void visit(Conv_Op_ref) {}
  void visit(Allocate_ref) {}
  void visit(Store_ref) {}
  void visit(Abstract_Memory_ref) {}
  void visit(MemCpy_ref) {}
  void visit(MemMove_ref) {}
  void visit(MemSet_ref) {}
  void visit(Return_Value_ref) {}
  void visit(Unreachable_ref) {}
  void visit(VA_Start_ref) {}
  void visit(VA_End_ref) {}
  void visit(VA_Copy_ref) {}

}; // end class stmt_replace_internal_vars

} // end namespace transformations
} // end namespace arbos

namespace arbos {
namespace transformations {
// Return a sequence of statements to compute the initial value of gv.
// The list can be empty if no initial value is available.
inline StmtRange getInitialization(Global_Variable_ref gv) {
  // PRE: we are assuming one single block and entry and exit are
  // the same block.
  Code_ref initializer = ar::getInitializer(gv);
  if (ar::ar_internal::is_null_ref(initializer)) {
    return StmtRange();
  }
  boost::optional< Basic_Block_ref > entry = ar::getEntryBlock(initializer);
  boost::optional< Basic_Block_ref > exit = ar::getExitBlock(initializer);
  if (!entry || !exit || !(*entry == *exit)) {
    assert(false && "Unexpected initialization code for a global variable");
    return StmtRange();
  }
  return ar::getStatements(*entry);
}

} // end namespace transformations

} // end namespace arbos

#endif // ANALYZER_TRANSFORMATIONS_HPP
