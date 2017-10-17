/*******************************************************************************
 *
 * Implementation of the ARBOS classes
 *
 * Authors: Nija Shi
 *          Arnaud J. Venet
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

#include <fstream>
#include <iostream>
#include <map>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>

#include <boost/optional.hpp>

#include <arbos/semantics/ar.hpp>

using namespace arbos;

// UID starts with 1; 0 represents an invalid UID.
index64_t UIDGenerator::currentUID = 1;

// ReferenceCounter
ReferenceCounter* ReferenceCounter::_instance = nullptr;

////
// AR_Node
////

AR_Node::AR_Node() : _uid(UIDGenerator::nextUID()) {
  std::unique_ptr< AR_Node > ptr(this);
  ARModel::Instance()->registerARNode(_uid, std::move(ptr));
}

////
// AR_Type
////

AR_Type::AR_Type() : _real_size(0), _store_size(0) {}

AR_Type::AR_Type(z_number real_size, z_number store_size)
    : _real_size(real_size), _store_size(store_size) {}

std::string AR_Type::getSignednessText(Signedness signedness) {
  switch (signedness) {
    case sign_unknown:
      return "sign_unknown";
    case sign_unsigned:
      return "sign_unsigned";
    case sign_signed:
      return "sign_signed";
    default:
      return 0;
  }
}

////
// AR_Function_Type
////

AR_Function_Type::AR_Function_Type(
    AR_Node_Ref< AR_Type > return_type,
    const std::vector< AR_Node_Ref< AR_Type > >& param_types,
    bool is_varargs)
    : _return_type(return_type),
      _param_types(param_types),
      _isVARARGS(is_varargs) {}

AR_Function_Type::AR_Function_Type(s_expression e) : _isVARARGS(false) {
  /**
   * s-expr: ($function ($return_ty (!34)) ($params_ty (!48) (!25)) ($isVarArg
   * ($False)))
   */
  s_expression_ref retty, params_ty, isVA;
  if (s_pattern("function",
                s_pattern("return_ty", retty),
                params_ty,
                s_pattern("isVarArg", isVA)) ^
      e) {
    index64_t ret_ty_id = (static_cast< index64_atom& >(**retty)).data();
    _return_type = ARFactory::createType(ret_ty_id);
    _param_types.reserve(params_ty->n_args());

    for (size_t i = 1; i <= params_ty->n_args(); i++) {
      s_expression p = (*params_ty)[i];
      index64_t id = (static_cast< index64_atom& >(*p)).data();
      _param_types.push_back(ARFactory::createType(id));
    }

    std::string isVAStr = (static_cast< string_atom& >(**isVA)).data();
    if (isVAStr == "True") {
      _isVARARGS = true;
    } else if (isVAStr == "False") {
      _isVARARGS = false;
    } else {
      throw parse_error("AR_Function_Type::isVARARGS", *isVA);
    }
  } else {
    throw parse_error("AR_Function_Type", e);
  }
}

AR_Function_Type::~AR_Function_Type() {}

void AR_Function_Type::print(std::ostream& out) {
  (*_return_type).print(out);

  out << " ";

  auto it = _param_types.begin();
  out << "(FuncTy!" << getUID() << ") (";
  while (it != _param_types.end()) {
    (**it).print(out);
    it++;
    if (it != _param_types.end())
      out << ", ";
  }
  out << " isVarArgs=" << (isVarargs() ? "True" : "False") << ")";
}

////
// AR_Integer_Type
////

AR_Integer_Type::AR_Integer_Type(z_number real_size,
                                 z_number store_size,
                                 Signedness signedness)
    : AR_Type(real_size, store_size), _signedness(signedness) {}

AR_Integer_Type::AR_Integer_Type(s_expression e)
    : AR_Type(0, 0), _signedness(sign_unknown) {
  /**
   * s-expr: ($int (#8) ($realsize (#8)) ($storesize (#1)))
   */
  s_expression_ref rs, ss;
  if (s_pattern("int", s_pattern("realsize", rs), s_pattern("storesize", ss)) ^
      e) {
    _real_size = (static_cast< z_number_atom& >(**rs)).data();
    _store_size = (static_cast< z_number_atom& >(**ss)).data();
  } else {
    throw parse_error("AR_Integer_Type", e);
  }
}

void AR_Integer_Type::print(std::ostream& out) {
  out << "i" << _real_size;
}

////
// AR_FP_Type
////

AR_FP_Type::AR_FP_Type(z_number real_size,
                       z_number store_size,
                       Signedness signedness,
                       const std::string& fp_kind)
    : AR_Type(real_size, store_size),
      _signedness(signedness),
      _fp_kind(fp_kind) {}

AR_FP_Type::AR_FP_Type(s_expression e)
    : AR_Type(0, 0), _signedness(sign_unknown) {
  /**
   * s-expr: ($float ($double) ($realsize (#64)) ($storesize (#8)))
   */
  s_expression_ref fp, rs, ss;
  if (s_pattern("float",
                fp,
                s_pattern("realsize", rs),
                s_pattern("storesize", ss)) ^
      e) {
    _fp_kind = (static_cast< string_atom& >(**fp)).data();
    _real_size = (static_cast< z_number_atom& >(**rs)).data();
    _store_size = (static_cast< z_number_atom& >(**ss)).data();
  } else {
    throw parse_error("AR_FP_Type", e);
  }
}

void AR_FP_Type::print(std::ostream& out) {
  out << _fp_kind << "_" << _real_size;
}

////
// AR_Structure_Type
////

const std::string AR_Structure_Type::ALIGN_TAG = "$align";
const std::string AR_Structure_Type::SIZE_TAG = "$size";

AR_Structure_Type::AR_Structure_Type(
    z_number real_size,
    z_number store_size,
    z_number alignment,
    const std::map< z_number, AR_Node_Ref< AR_Type > >& layout)
    : AR_Type(real_size, store_size), _alignment(alignment) {
  _field_types.reserve(layout.size());
  for (const auto& e : layout) {
    _layout[e.first] = e.second.getUID();
    _field_types.push_back(e.second.getUID());
  }
}

AR_Structure_Type::AR_Structure_Type(s_expression e) {
  /**
   * s-expr: ($struct ($layout (#0 (!16))) ($align (#64)) ($realsize (#128))
   * ($storesize (#16)))
   */
  s_expression_ref l, a, rs, ss;
  if (s_pattern("struct",
                l,
                s_pattern("align", a),
                s_pattern("realsize", rs),
                s_pattern("storesize", ss)) ^
      e) {
    s_expression layout_s = *l;
    for (size_t i = 1; i <= layout_s.n_args(); i++) {
      s_expression l = layout_s[i];
      z_number offset = (static_cast< z_number_atom& >(*l)).data();
      index64_t ref = (static_cast< index64_atom& >(*l[1])).data();
      AR_Node_Ref< AR_Type > type = ARFactory::createType(ref);
      _layout[offset] = type.getUID();
      _field_types.push_back(type.getUID());
    }
    _alignment = (static_cast< z_number_atom& >(**a)).data();
    _real_size = (static_cast< z_number_atom& >(**rs)).data();
    _store_size = (static_cast< z_number_atom& >(**ss)).data();
  } else {
    throw parse_error("AR_Structure_Type", e);
  }
}

AR_Structure_Type::~AR_Structure_Type() {}

std::pair< type_iterator, type_iterator > AR_Structure_Type::
    getAllFieldTypes() {
  return std::make_pair(boost::make_transform_iterator(_field_types.begin(),
                                                       MkNodeRef< AR_Type >()),
                        boost::make_transform_iterator(_field_types.end(),
                                                       MkNodeRef< AR_Type >()));
}

void AR_Structure_Type::print(std::ostream& out) {
  out << "struct!" << getUID();
}

void AR_Structure_Type::print_layout(std::vector< index64_t >& visited,
                                     std::ostream& out) {
  if (find(visited.begin(), visited.end(), getUID()) != visited.end()) {
    out << "struct!" << getUID();
    return;
  }

  out << "struct!" << getUID() << "{";
  std::map< z_number, index64_t >::iterator p = _layout.begin();
  while (p != _layout.end()) {
    out << "offset=" << p->first << ", ty=";
    AR_Node_Ref< AR_Type > ty(p->second);
    (*ty).print(out);
    p++;
    if (p != _layout.end()) {
      out << "; ";
    }
  }
  out << "}";
}

z_number AR_Structure_Type::getOffsetOfField(int field_position) {
  std::map< z_number, index64_t >::iterator p = _layout.begin();
  int ct = field_position;
  while (ct > 0) {
    ct--;
    p++;
  }
  return p->first;
}

z_number AR_Structure_Type::getSizeOfField(int field_position) {
  std::map< z_number, index64_t >::iterator p = _layout.begin();
  int ct = 0;
  while (p != _layout.end() && ct < field_position) {
    p++;
    ct++;
  }
  if (p != _layout.end()) {
    AR_Node_Ref< AR_Type > ty_ref(p->second);
    return (*ty_ref).getRealSize();
  } else {
    return -1;
  }
}

////
// AR_Pointer_Type
////

AR_Pointer_Type::AR_Pointer_Type(z_number real_size,
                                 z_number store_size,
                                 AR_Node_Ref< AR_Type > cell_type)
    : AR_Type(real_size, store_size), _cell_type(cell_type.getUID()) {}

AR_Pointer_Type::AR_Pointer_Type(s_expression e)
    : AR_Type(0, 0), _cell_type(0) {
  /**
   * Example s-expr: ($ptr ($ty (!18)) ($realsize (#64)) ($storesize (#8)))
   */
  s_expression_ref id, rs, ss;
  if (s_pattern("ptr",
                s_pattern("ty", id),
                s_pattern("realsize", rs),
                s_pattern("storesize", ss)) ^
      e) {
    _real_size = (static_cast< z_number_atom& >(**rs)).data();
    _store_size = (static_cast< z_number_atom& >(**ss)).data();
    index64_t ref = (static_cast< index64_atom& >(**id)).data();
    _cell_type = ARFactory::createType(ref).getUID();
  } else {
    throw parse_error("AR_Pointer_Type", e);
  }
}

AR_Pointer_Type::~AR_Pointer_Type() {}

void AR_Pointer_Type::print(std::ostream& out) {
  AR_Node_Ref< AR_Type > ty_ref(_cell_type);
  (*ty_ref).print(out);
  out << "*";
}

////
// AR_Array_Type
////

AR_Array_Type::AR_Array_Type(z_number real_size,
                             z_number store_size,
                             AR_Node_Ref< AR_Type > cell_type,
                             z_number capacity)
    : AR_Type(real_size, store_size),
      _cell_type(cell_type.getUID()),
      _capacity(capacity) {}

AR_Array_Type::AR_Array_Type(s_expression e)
    : AR_Type(0, 0), _cell_type(0), _capacity(0) {
  /**
   * Example s-expr: ($array ($len (#0)) ($ty (!23)) ($realsize (#0))
   * ($storesize (#0)))
   */
  s_expression_ref len, ty, rs, ss;
  if (s_pattern("array",
                s_pattern("len", len),
                s_pattern("ty", ty),
                s_pattern("realsize", rs),
                s_pattern("storesize", ss)) ^
      e) {
    _capacity = (static_cast< z_number_atom& >(**len)).data();
    index64_t ty_id = (static_cast< index64_atom& >(**ty)).data();
    AR_Node_Ref< AR_Type > cell_type_ref = ARFactory::createType(ty_id);
    _cell_type = cell_type_ref.getUID();
    _real_size = (static_cast< z_number_atom& >(**rs)).data();
    _store_size = (static_cast< z_number_atom& >(**ss)).data();
  } else {
    throw parse_error("AR_Array_Type", e);
  }
}

AR_Array_Type::~AR_Array_Type() {}

void AR_Array_Type::print(std::ostream& out) {
  out << "[" << _capacity << " x ";
  AR_Node_Ref< AR_Type > cell_type_ref(_cell_type);
  (*cell_type_ref).print(out);
  out << "]";
}

////
// AR_Void_Type
////

AR_Void_Type::AR_Void_Type() {}

AR_Void_Type::AR_Void_Type(s_expression e) {
  /**
   * ($void ($realsize (#0)) ($storesize) (#0))
   */
  s_expression_ref rs, ss;
  if (s_pattern("void", s_pattern("realsize", rs), s_pattern("storesize", ss)) ^
      e) {
    _real_size = (static_cast< z_number_atom& >(**rs)).data();
    _store_size = (static_cast< z_number_atom& >(**ss)).data();
  } else {
    throw parse_error("AR_Void_Type", e);
  }
}

void AR_Void_Type::print(std::ostream& out) {
  out << "void !" << getUID();
}

////
// AR_Opaque_Type
////

AR_Opaque_Type::AR_Opaque_Type(z_number store_size)
    : AR_Type(store_size, store_size) {}

void AR_Opaque_Type::print(std::ostream& out) {
  out << "opaque !" << getUID();
}

/**
 * AR_Undefined_Constant
 */

AR_Undefined_Constant::AR_Undefined_Constant(AR_Node_Ref< AR_Type > type)
    : _type(type) {}

AR_Undefined_Constant::AR_Undefined_Constant(s_expression e)
    : _type(AR_Null_Ref) {
  /**
   * Example s-expr: ($undefined ($ty (!11)))
   */
  s_expression_ref ty;
  if (s_pattern("undefined", s_pattern("ty", ty)) ^ e) {
    index64_t ty_id = (static_cast< index64_atom& >(**ty)).data();
    _type = ARFactory::createType(ty_id);
  } else {
    throw parse_error("AR_Undefined", e);
  }
}

void AR_Undefined_Constant::print(std::ostream& out) {
  out << "undefined:";
  (*_type).print(out);
}

////
// AR_Integer_Constant
////

AR_Integer_Constant::AR_Integer_Constant(AR_Node_Ref< AR_Integer_Type > type,
                                         z_number value)
    : _type(type), _value(value) {}

AR_Integer_Constant::AR_Integer_Constant(s_expression e) : _type(AR_Null_Ref) {
  /**
   * Example s-expr: ($constantint ($val (#0)) ($ty (!2)))
   */
  s_expression v = e[1];
  _value = (static_cast< z_number_atom& >(*v[1])).data();

  s_expression ty = e[2];
  index64_t ty_id = (static_cast< index64_atom& >(*ty[1])).data();
  _type = node_cast< AR_Integer_Type >(ARFactory::createType(ty_id));
}

AR_Integer_Constant::~AR_Integer_Constant() {}

void AR_Integer_Constant::print(std::ostream& out) {
  out << _value << ":";
  (*_type).print(out);
}

////
// AR_FP_Constant
////

AR_FP_Constant::AR_FP_Constant(AR_Node_Ref< AR_FP_Type > type, fp_number value)
    : _type(type), _value(value) {}

AR_FP_Constant::AR_FP_Constant(s_expression e) : _type(AR_Null_Ref) {
  /**
   * Example s-expr: ($constantfp ($val (^0.88)) ($ty (!4)))
   */
  s_expression_ref v, ty;
  if (s_pattern("constantfp", v, ty) ^ e) {
    _value = (static_cast< fp_number_atom& >(*(*v)[1])).data();
    index64_t id = ((static_cast< index64_atom& >(*(*ty)[1]))).data();
    _type = node_cast< AR_FP_Type >(ARFactory::createType(id));
  } else {
    throw parse_error("AR_FP_Constant", e);
  }
}

AR_FP_Constant::~AR_FP_Constant() {}

void AR_FP_Constant::print(std::ostream& out) {
  out << _value << ":";
  (*_type).print(out);
}

////
// AR_Null_Constant
////

AR_Null_Constant::AR_Null_Constant(AR_Node_Ref< AR_Type > type) : _type(type) {}

AR_Null_Constant::AR_Null_Constant(s_expression e) : _type(AR_Null_Ref) {
  /**
   * Example s-expr: ($constantpointernull ($ty (!22)))
   */
  s_expression_ref ty;
  if (s_pattern("constantpointernull", s_pattern("ty", ty)) ^ e) {
    index64_t ty_id = (static_cast< index64_atom& >(**ty)).data();
    _type = ARFactory::createType(ty_id);
  } else {
    throw parse_error("AR_Null_Constant", e);
  }
}

AR_Null_Constant::~AR_Null_Constant() {}

void AR_Null_Constant::print(std::ostream& out) {
  out << "null:";
  (*_type).print(out);
}

////
// AR_Range_Constant
////

AR_Range_Constant::AR_Range_Constant(
    AR_Node_Ref< AR_Type > type,
    const std::map< z_number, AR_Node_Ref< AR_Operand > >& values)
    : _type(type), _values(values) {}

AR_Range_Constant::AR_Range_Constant(s_expression e) : _type(AR_Null_Ref) {
  /**
   * Example s-expr: ($range
   ($values
   ($value (#0) ($cst ($constantfp ($val (^0x0p0)) ($ty (!4)))))
   ($value (#64) ($cst ($constantfp ($val (^0x0p0)) ($ty (!4)))))
   ($value (#128) ($cst ($constantfp ($val (^0x0p0)) ($ty (!4)))))
   ($value (#192) ($cst ($constantfp ($val (^0x0p0)) ($ty (!4)))))
   ($value (#256) ($cst ($constantfp ($val (^0x0p0)) ($ty (!4)))))
   ($value (#320) ($cst ($constantfp ($val (^0x0p0)) ($ty (!4)))))
   ($value (#384) ($cst ($constantfp ($val (^0x0p0)) ($ty (!4)))))
   ($value (#448) ($cst ($constantfp ($val (^0x0p0)) ($ty (!4)))))
   ($value (#512) ($cst ($constantfp ($val (^0x0p0)) ($ty (!4)))))
   ($value (#576) ($cst ($constantfp ($val (^0x0p0)) ($ty (!4)))))
   ($ty (!7))))
   */
  s_expression_ref v, ty;
  if (s_pattern("range", v, s_pattern("ty", ty)) ^ e) {
    for (size_t i = 1; i <= (*v).n_args(); i++) {
      s_expression_ref o, cst;
      if (s_pattern("value", o, cst) ^ (*v)[i]) {
        z_number offset = (static_cast< z_number_atom& >(**o)).data();
        _values.insert(std::make_pair(offset, ARFactory::createOperand(*cst)));
      } else {
        throw parse_error("AR_Range_Constant::values", (*v)[i]);
      }
    }

    index64_t ty_id = (static_cast< index64_atom& >(**ty)).data();
    _type = ARFactory::createType(ty_id);
  } else {
    throw parse_error("AR_Range_Constant", e);
  }
}

void AR_Range_Constant::print(std::ostream& out) {
  out << "range {";
  std::map< z_number, AR_Node_Ref< AR_Operand > >::iterator p = _values.begin();
  for (; p != _values.end(); p++) {
    out << "(" << p->first << ", ";
    (*p->second).print(out);
    out << ")";
  }
  out << "}";
}

AR_Range_Constant::~AR_Range_Constant() {}

////
// AR_Var_Addr_Constant
////

AR_Var_Addr_Constant::AR_Var_Addr_Constant(AR_Node_Ref< AR_Variable > variable)
    : _variable(variable.getUID()) {}

AR_Var_Addr_Constant::AR_Var_Addr_Constant(s_expression e) : _variable(0) {
  /**
   * Example s-expr:
   ($globalvariableref ($name ($a)) ($gv (!2)) ($ty (!7))), or
   ($localvariableref ($name ($a)) ($ty (!11))))
   */
  s_expression_ref gv, lv_name;
  if (s_pattern("globalvariableref",
                s_pattern(),
                s_pattern("gv", gv),
                s_pattern()) ^
      e) {
    index64_t id = (static_cast< index64_atom& >(**gv)).data();
    _variable = ARFactory::createGlobalVariable(id).getUID();
  } else if (s_pattern("localvariableref",
                       s_pattern("name", lv_name),
                       s_pattern()) ^
             e) {
    std::string name = (static_cast< string_atom& >(**lv_name)).data();

    // Find the variable in the set of local variables
    _variable = (*ARModel::Instance()->getCurrentBuildingFunction())
                    .getLocalVariable(name)
                    .getUID();

    // If it's not there, then try to find in the set of parameters
    if (_variable == 0) {
      _variable = (*ARModel::Instance()->getCurrentBuildingFunction())
                      .getFormalParameter(name)
                      .getUID();
    }

    // Error: cannot find the variable
    if (_variable == 0) {
      throw parse_error("AR_Var_Addr_Constant cannot find local var", e);
    }
  } else {
    throw parse_error("AR_Var_Addr_Constant", e);
  }
}

AR_Node_Ref< AR_Type > AR_Var_Addr_Constant::getType() {
  return (*getVariable()).getType();
}

AR_Var_Addr_Constant::~AR_Var_Addr_Constant() {}

void AR_Var_Addr_Constant::print(std::ostream& out) {
  out << "vaddr(";
  AR_Node_Ref< AR_Variable > var(_variable);
  (*var).print(out);
  out << ") !" << getUID();
}

////
// AR_Function_Addr_Constant
////

AR_Function_Addr_Constant::AR_Function_Addr_Constant(
    AR_Node_Ref< AR_Type > type, const std::string& function_name)
    : _type(type), _function_name(function_name) {}

AR_Function_Addr_Constant::AR_Function_Addr_Constant(s_expression e)
    : _type(AR_Null_Ref) {
  /**
   * Example s-expr: ($constantfunctionptr ($name
   * ($llvm\5C2Ememset\5C2Ep0i8\5C2Ei64)) ($ty (!14)))
   */
  s_expression_ref n, ty;
  if (s_pattern("constantfunctionptr",
                s_pattern("name", n),
                s_pattern("ty", ty)) ^
      e) {
    _function_name = (static_cast< string_atom& >(**n)).data();
    index64_t ty_id = (static_cast< index64_atom& >(**ty)).data();
    _type = ARFactory::createType(ty_id);
  } else {
    throw parse_error("AR_Function_Addr_Constant", e);
  }
}

AR_Node_Ref< AR_Function > AR_Function_Addr_Constant::getFunction() {
  AR_Node_Ref< AR_Function > func_ref(
      ARModel::Instance()->getFunctionUIDByNameId(_function_name));
  return func_ref;
}

AR_Function_Addr_Constant::~AR_Function_Addr_Constant() {}

void AR_Function_Addr_Constant::print(std::ostream& out) {
  AR_Node_Ref< AR_Function > func_ref(
      ARModel::Instance()->getFunctionUIDByNameId(_function_name));
  out << "faddr(" << _function_name << " !" << func_ref.getUID() << ") !"
      << getUID();
}

////
// AR_Variable
////

AR_Variable::AR_Variable()
    : _type(AR_Null_Ref), _source_location(AR_Null_Ref) {}

AR_Variable::AR_Variable(AR_Node_Ref< AR_Type > type,
                         const std::string& name,
                         AR_Node_Ref< AR_Source_Location > source_location)
    : _type(type), _name(name), _source_location(source_location) {}

AR_Variable::~AR_Variable() {}

void AR_Variable::print_sourcelocation(std::ostream& out) {
  if (_source_location.getUID() > 0) {
    out << " ";
    (*_source_location).print(out);
  }
}

////
// AR_Global_Variable
////

AR_Global_Variable::AR_Global_Variable(
    AR_Node_Ref< AR_Type > type,
    const std::string& name,
    AR_Node_Ref< AR_Source_Location > source_location,
    AR_Node_Ref< AR_Code > initializer)
    : AR_Variable(type, name, source_location), _initializer(initializer) {}

AR_Global_Variable::AR_Global_Variable(s_expression e)
    : _initializer(AR_Null_Ref) {
  /**
   * Example s-expr: ($gv ($name ($<name>)) ($ty (!<ty_id>)) ($code) ($debug
   * ($srcloc)))
   */
  s_expression_ref n, ty, align, init_code, srcloc;
  if (s_pattern("gv",
                s_pattern("name", n),
                s_pattern("ty", ty),
                align,
                init_code,
                s_pattern("debug", srcloc)) ^
      e) {
    _name = (static_cast< string_atom& >(**n)).data();
    index64_t ty_id = (static_cast< index64_atom& >(**ty)).data();
    _type = ARFactory::createType(ty_id);
    if ((*align).n_args() > 0) {
      s_expression align_s_expr = *align;
      s_pattern("align", align) ^ align_s_expr;
      _alignment = (static_cast< z_number_atom& >(**align)).data();
    }
    if ((*init_code).n_args() > 0) {
      _initializer =
          ARFactory::createCode(0, *init_code); // Synthesized function for
                                                // initializing global variables
    }
    _source_location = ARFactory::createSourceLocation(*srcloc);
  } else {
    throw parse_error("AR_Global_Variable", e);
  }
}

AR_Global_Variable::~AR_Global_Variable() {}

void AR_Global_Variable::print(std::ostream& out) {
  out << "def gv!" << getUID();
  out << _name << ":";
  (*_type).print(out);
  if (_alignment)
    out << ", align " << *_alignment;
  print_sourcelocation(out);
}

////
// AR_Local_Variable
////

AR_Local_Variable::AR_Local_Variable(
    AR_Node_Ref< AR_Type > type,
    const std::string& name,
    AR_Node_Ref< AR_Source_Location > source_location)
    : AR_Variable(type, name, source_location) {}

AR_Local_Variable::AR_Local_Variable(s_expression e) {
  /**
   * Example s-expr: ($local_var ($var ($name ($a)) ($ty (!11))))
   */
  s_expression_ref n, ty, align;
  if (s_pattern("local_var",
                s_pattern("var",
                          s_pattern("name", n),
                          s_pattern("ty", ty),
                          align)) ^
      e) {
    _name = (static_cast< string_atom& >(**n)).data();
    index64_t ty_id = (static_cast< index64_atom& >(**ty)).data();
    _type = ARFactory::createType(ty_id);
    if ((*align).n_args() > 0) {
      s_expression align_s_expr = *align;
      s_pattern("align", align) ^ align_s_expr;
      _alignment = (static_cast< z_number_atom& >(**align)).data();
    }
  } else {
    throw parse_error("AR_Local_Variable", e);
  }
}

AR_Local_Variable::~AR_Local_Variable() {}

void AR_Local_Variable::print(std::ostream& out) {
  out << "def localvar!" << getUID() << " " << _name << ":";
  if (_alignment)
    out << ", align " << *_alignment;
  (*_type).print(out);
}

////
// AR_Cst_Operand
////

AR_Cst_Operand::AR_Cst_Operand(AR_Node_Ref< AR_Constant > constant)
    : _constant(constant) {}

AR_Cst_Operand::AR_Cst_Operand(s_expression e) : _constant(AR_Null_Ref) {
  /**
   * Example s-expr: ($cst ($constantint ($val (#0)) ($ty (!6))))
   */
  s_expression_ref cst;
  if (s_pattern("cst", cst) ^ e) {
    _constant = ARFactory::createConstant(*cst);
  } else {
    throw parse_error("AR_Cst_Operand", e);
  }
}

AR_Cst_Operand::~AR_Cst_Operand() {}

void AR_Cst_Operand::print(std::ostream& out) {
  (*_constant).print(out);
}

////
// AR_Var_Operand
////

AR_Var_Operand::AR_Var_Operand(
    AR_Node_Ref< AR_Internal_Variable > internal_variable)
    : _internal_variable(internal_variable) {}

AR_Var_Operand::AR_Var_Operand(s_expression e)
    : _internal_variable(AR_Null_Ref) {
  /**
   * Example s-expr: ($var ($name ($i.0)) ($ty (!2))))
   */
  _internal_variable = ARFactory::createInternalVariable(e);
}

AR_Var_Operand::~AR_Var_Operand() {}

void AR_Var_Operand::print(std::ostream& out) {
  (*_internal_variable).print(out);
}

////
// AR_Internal_Variable
////

AR_Internal_Variable::AR_Internal_Variable(AR_Node_Ref< AR_Type > type,
                                           const std::string& name)
    : _type(type), _name(name) {}

AR_Internal_Variable::AR_Internal_Variable(s_expression e)
    : _type(AR_Null_Ref) {
  /**
   * Example s-expr: ($var ($name ($tmp)) ($ty (!11)))
   */
  s_expression_ref n, ty;
  if (s_pattern("var", s_pattern("name", n), s_pattern("ty", ty)) ^ e) {
    _name = (static_cast< string_atom& >(**n)).data();
    index64_t ty_id = (static_cast< index64_atom& >(**ty)).data();
    _type = ARFactory::createType(ty_id);
  } else {
    throw parse_error("AR_Internal_Variable", e);
  }
}

AR_Internal_Variable::~AR_Internal_Variable() {}

void AR_Internal_Variable::print(std::ostream& out) {
  out << _name << ":";
  (*_type).print(out);
}

////
// AR_Statement
////

AR_Statement::AR_Statement()
    : _parent_bblock_uid(0), _source_location(AR_Null_Ref) {}

AR_Statement::AR_Statement(AR_Node_Ref< AR_Source_Location > source_location)
    : _parent_bblock_uid(0), _source_location(source_location) {}

AR_Statement::AR_Statement(index64_t parent_bblock_uid)
    : _parent_bblock_uid(parent_bblock_uid), _source_location(AR_Null_Ref) {}

AR_Statement::~AR_Statement() {}

void AR_Statement::print_sourcelocation(std::ostream& out) {
  if (_source_location.getUID() > 0) {
    out << " ";
    (*_source_location).print(out);
  }
}

////
// AR_Integer_Comparison
////

const std::string TRUE_STR = "true";
const std::string FALSE_STR = "false";

AR_Integer_Comparison::AR_Integer_Comparison(
    CompOp predicate,
    bool continue_if_true,
    AR_Node_Ref< AR_Operand > left_op,
    AR_Node_Ref< AR_Operand > right_op,
    AR_Node_Ref< AR_Source_Location > source_loc)
    : AR_Statement(source_loc),

      _predicate(predicate),
      _continue_if_true(continue_if_true),
      _left_op(left_op),
      _right_op(right_op) {}

AR_Integer_Comparison::AR_Integer_Comparison(index64_t parent_bblock_uid,
                                             s_expression e)
    : AR_Statement(parent_bblock_uid),
      _left_op(AR_Null_Ref),
      _right_op(AR_Null_Ref) {
  // Example code: ($cmp ($cond ($true)) ($pred ($sle)) ($ops ($namedvar ($name
  // ($i\2E0)) ($ty (!11))) ($constant ($cst (!22)) ($ty (!11)))))
  s_expression_ref c, pred, operands;
  if (s_pattern("cmp",
                s_pattern("cond", c),
                s_pattern("pred", pred),
                operands) ^
      e) {
    std::string cond = (static_cast< string_atom& >(**c)).data();
    if (cond == TRUE_STR) {
      _continue_if_true = true;
    } else if (cond == FALSE_STR) {
      _continue_if_true = false;
    } else {
      throw error("AR_Integer_Comparison: invalid boolean value \"" + cond +
                  "\" for continue_if_true");
    }

    _predicate = translate((static_cast< string_atom& >(**pred)).data());
    _left_op = ARFactory::createOperand((*operands)[1]);
    _right_op = ARFactory::createOperand((*operands)[2]);
  } else {
    throw parse_error("AR_Integer_Comparison", e);
  }
}

void AR_Integer_Comparison::print(std::ostream& out) {
  out << "assert ";
  out << (_continue_if_true ? "true" : "false");
  out << " " << getCompOpText() << "(";
  (*_left_op).print(out);
  out << ", ";
  (*_right_op).print(out);
  out << ")";
}

std::string AR_Integer_Comparison::getCompOpText() {
  switch (_predicate) {
    case eq:
      return "eq";
    case ne:
      return "ne";
    case ult:
      return "ult";
    case ule:
      return "ule";
    case slt:
      return "slt";
    case sle:
      return "sle";
    default:
      return EM_STR;
  }
}

CompOp AR_Integer_Comparison::translate(const std::string& pred) {
  if (pred == "eq") {
    return eq;
  } else if (pred == "ne") {
    return ne;
  } else if (pred == "ult") {
    return ult;
  } else if (pred == "ule") {
    return ule;
  } else if (pred == "slt") {
    return slt;
  } else if (pred == "sle") {
    return sle;
  } else {
    throw error("Unsupported integer comparison operator \"" + pred + "\"");
  }
}

////
// AR_FP_Comparison
////

AR_FP_Comparison::AR_FP_Comparison(FP_CompOp predicate,
                                   bool continue_if_true,
                                   AR_Node_Ref< AR_Operand > left_op,
                                   AR_Node_Ref< AR_Operand > right_op,
                                   AR_Node_Ref< AR_Source_Location > source_loc)
    : AR_Statement(source_loc),
      _predicate(predicate),
      _continue_if_true(continue_if_true),
      _left_op(left_op),
      _right_op(right_op) {}

AR_FP_Comparison::AR_FP_Comparison(index64_t parent_bblock_uid, s_expression e)
    : AR_Statement(parent_bblock_uid),
      _left_op(AR_Null_Ref),
      _right_op(AR_Null_Ref) {
  // Example code: ($cmp ($cond ($true)) ($pred ($sle)) ($ops ($namedvar ($name
  // ($i\2E0)) ($ty (!11))) ($constant ($cst (!22)) ($ty (!11)))))
  s_expression_ref c, pred, operands;
  if (s_pattern("cmp",
                s_pattern("cond", c),
                s_pattern("pred", pred),
                operands) ^
      e) {
    std::string cond = (static_cast< string_atom& >(**c)).data();

    if (cond == TRUE_STR) {
      _continue_if_true = true;
    } else if (cond == FALSE_STR) {
      _continue_if_true = false;
    } else {
      throw error("AR_Integer_Comparison: invalid boolean value \"" + cond +
                  "\" for continue_if_true");
    }

    _predicate = translate((static_cast< string_atom& >(**pred)).data());
    _left_op = ARFactory::createOperand((*operands)[1]);
    _right_op = ARFactory::createOperand((*operands)[2]);
  } else {
    throw parse_error("AR_Integer_Comparison", e);
  }
}

void AR_FP_Comparison::print(std::ostream& out) {
  out << "assert ";
  out << (_continue_if_true ? "true" : "false");
  out << " " << getCompOpText() << "(";
  (*_left_op).print(out);
  out << ", ";
  (*_right_op).print(out);
  out << ")";
}

std::string AR_FP_Comparison::getCompOpText() {
  switch (_predicate) {
    case oeq:
      return "oeq";
    case olt:
      return "olt";
    case ole:
      return "ole";
    case one:
      return "one";
    case ord:
      return "ord";
    case ueq:
      return "ueq";
    case _ult:
      return "ult";
    case _ule:
      return "ule";
    case une:
      return "une";
    case uno:
      return "uno";
    default:
      return EM_STR;
  }
}

FP_CompOp AR_FP_Comparison::translate(const std::string& pred) {
  if (pred == "oeq") {
    return oeq;
  } else if (pred == "olt") {
    return olt;
  } else if (pred == "ole") {
    return ole;
  } else if (pred == "one") {
    return one;
  } else if (pred == "ord") {
    return ord;
  } else if (pred == "ueq") {
    return ueq;
  } else if (pred == "ult") {
    return _ult;
  } else if (pred == "ule") {
    return _ule;
  } else if (pred == "une") {
    return une;
  } else if (pred == "uno") {
    return uno;
  } else {
    throw error("Unsupported FP comparison operator");
  }
}

////
// AR_Arith_Op
////

AR_Arith_Op::AR_Arith_Op(ArithOp op,
                         AR_Node_Ref< AR_Internal_Variable > result,
                         AR_Node_Ref< AR_Operand > left_op,
                         AR_Node_Ref< AR_Operand > right_op,
                         AR_Node_Ref< AR_Source_Location > source_loc)
    : AR_Statement(source_loc),
      _op(op),
      _result(result),
      _left_op(left_op),
      _right_op(right_op) {}

AR_Arith_Op::AR_Arith_Op(index64_t parent_bblock, s_expression e, ArithOp op)
    : AR_Statement(parent_bblock),
      _op(op),
      _result(AR_Null_Ref),
      _left_op(AR_Null_Ref),
      _right_op(AR_Null_Ref) {
  /**
   * Example s-expr: ($mul
   ($lhs ($var ($name ($__v:43)) ($ty (!16))))
   ($rhs ($cst ($constantint ($val (#3)) ($ty (!16)))) ($cst ($constantint ($val
   (#0)) ($ty (!16)))))
   ($debug ($srcloc ($line (#-1)) ($file (!17)))))
   */
  s_expression_ref lhs, lo, ro, ty, srcloc;
  std::string opname = getArithOpText();
  if (s_pattern(opname,
                s_pattern("lhs", lhs),
                s_pattern("rhs", lo, ro),
                s_pattern("debug", srcloc)) ^
      e) {
    _result = ARFactory::createInternalVariable(*lhs);
    _left_op = ARFactory::createOperand(*lo);
    _right_op = ARFactory::createOperand(*ro);

    // Get filename and line number
    _source_location = ARFactory::createSourceLocation(*srcloc);
  } else {
    throw parse_error("AR_Arith_Op", e);
  }
}

AR_Arith_Op::~AR_Arith_Op() {}

void AR_Arith_Op::print(std::ostream& out) {
  (*_result).print(out);
  out << " = " << getArithOpText() << "(";
  (*_left_op).print(out);
  out << ", ";
  (*_right_op).print(out);
  out << ")";
}

std::string AR_Arith_Op::getArithOpText() {
  switch (_op) {
    case add:
      return "add";
    case sub:
      return "sub";
    case mul:
      return "mul";
    case udiv:
      return "udiv";
    case sdiv:
      return "sdiv";
    case urem:
      return "urem";
    case srem:
      return "srem";
    default:
      return EM_STR;
  }
}

////
// AR_Conv_Op
////

AR_Conv_Op::AR_Conv_Op(ConvOp op,
                       AR_Node_Ref< AR_Operand > result,
                       AR_Node_Ref< AR_Operand > operand,
                       AR_Node_Ref< AR_Source_Location > source_loc)
    : AR_Statement(source_loc), _op(op), _result(result), _operand(operand) {}

AR_Conv_Op::AR_Conv_Op(index64_t parent_bblock, s_expression e, ConvOp op)
    : AR_Statement(parent_bblock),
      _op(op),
      _result(AR_Null_Ref),
      _operand(AR_Null_Ref) {
  /**
   * Example code: ($sitofp
   ($lhs ($namedvar ($name ($tmp1)) ($ty (!4))))
   ($rhs ($namedvar ($name ($tmp)) ($ty (!11))))
   ($debug ($srcloc ($line (#9)) ($file (!6)))))
   */
  std::string opcode = getConvOpText(op);
  s_expression_ref lhs, rhs, srcloc;
  if (s_pattern(opcode,
                s_pattern("lhs", lhs),
                s_pattern("rhs", rhs),
                s_pattern("debug", srcloc)) ^
      e) {
    _result = ARFactory::createOperand(*lhs);
    _operand = ARFactory::createOperand(*rhs);
    _source_location = ARFactory::createSourceLocation(*srcloc);
  } else {
    throw parse_error("AR_Conv_Op", e);
  }
}

AR_Conv_Op::~AR_Conv_Op() {}

std::string AR_Conv_Op::getConvOpText(ConvOp op) {
  switch (op) {
    case trunc:
      return "trunc";
    case zext:
      return "zext";
    case sext:
      return "sext";
    case fptrunc:
      return "fptrunc";
    case fpext:
      return "fpext";
    case fptoui:
      return "fptoui";
    case fptosi:
      return "fptosi";
    case uitofp:
      return "uitofp";
    case sitofp:
      return "sitofp";
    case ptrtoint:
      return "ptrtoint";
    case inttoptr:
      return "inttoptr";
    case bitcast:
      return "bitcast";
    default:
      return EM_STR;
  }
}

void AR_Conv_Op::print(std::ostream& out) {
  (*_result).print(out);
  out << " = " << getConvOpText(_op) << "(";
  (*_operand).print(out);
  out << ")";
  print_sourcelocation(out);
}

////
// AR_Bitwise_Op
////

AR_Bitwise_Op::AR_Bitwise_Op(BitwiseOp op,
                             AR_Node_Ref< AR_Internal_Variable > result,
                             AR_Node_Ref< AR_Operand > left_op,
                             AR_Node_Ref< AR_Operand > right_op,
                             AR_Node_Ref< AR_Source_Location > source_loc)
    : AR_Statement(source_loc),
      _op(op),
      _result(result),
      _left_op(left_op),
      _right_op(right_op) {}

AR_Bitwise_Op::AR_Bitwise_Op(index64_t parent_bblock,
                             s_expression e,
                             BitwiseOp op)
    : AR_Statement(parent_bblock),
      _op(op),
      _result(AR_Null_Ref),
      _left_op(AR_Null_Ref),
      _right_op(AR_Null_Ref) {
  /**
   * Example s-expr: ($or ($lhs ($namedvar ($name ($tmp)) ($ty (!2)))) ($rhs
   * ($constant ($cst (!4)) ($ty (!2))) ($constant ($cst (!5)) ($ty (!2))))
   * ($debug ($srcloc ($line (#3)) ($file (!6)))))
   */
  std::string opcode = getBitwiseOpText();
  s_expression_ref lhs, rhs, srcloc;
  if (s_pattern(opcode,
                s_pattern("lhs", lhs),
                rhs,
                s_pattern("debug", srcloc)) ^
      e) {
    _result = ARFactory::createInternalVariable(*lhs);
    _left_op = ARFactory::createOperand((*rhs)[1]);
    _right_op = ARFactory::createOperand((*rhs)[2]);
    _source_location = ARFactory::createSourceLocation(*srcloc);
  } else {
    throw parse_error("AR_Bitwise_Op", e);
  }
}

AR_Bitwise_Op::~AR_Bitwise_Op() {}

void AR_Bitwise_Op::print(std::ostream& out) {
  (*_result).print(out);
  out << " = " << getBitwiseOpText() << "(";
  (*_left_op).print(out);
  out << ", ";
  (*_right_op).print(out);
  out << ")";
  print_sourcelocation(out);
}

std::string AR_Bitwise_Op::getBitwiseOpText() {
  switch (_op) {
    case _shl:
      return "shl";
    case _lshr:
      return "lshr";
    case _ashr:
      return "ashr";
    case _and:
      return "and";
    case _or:
      return "or";
    case _xor:
      return "xor";
    default:
      return EM_STR;
  }
}

////
// AR_FP_Op
////

AR_FP_Op::AR_FP_Op(FPOp op,
                   AR_Node_Ref< AR_Internal_Variable > result,
                   AR_Node_Ref< AR_Operand > left_op,
                   AR_Node_Ref< AR_Operand > right_op,
                   AR_Node_Ref< AR_Source_Location > source_loc)
    : AR_Statement(source_loc),
      _op(op),
      _result(result),
      _left_op(left_op),
      _right_op(right_op) {}

AR_FP_Op::AR_FP_Op(index64_t parent_bblock, s_expression e, FPOp op)
    : AR_Statement(parent_bblock),
      _op(op),
      _result(AR_Null_Ref),
      _left_op(AR_Null_Ref),
      _right_op(AR_Null_Ref) {
  /**
   * Example code: ($fmul ($lhs ($namedvar ($name ($tmp3)) ($ty (!4)))) ($rhs
   * ($namedvar ($name ($tmp1)) ($ty (!4))) ($namedvar ($name ($tmp2)) ($ty
   * (!4)))) ($debug ($srcloc ($line (#9)) ($file (!6)))))
   */
  s_expression_ref lhs, rhs, srcloc;
  std::string opcode = getFPOpText();
  if (s_pattern(opcode,
                s_pattern("lhs", lhs),
                rhs,
                s_pattern("debug", srcloc)) ^
      e) {
    _result = ARFactory::createInternalVariable(*lhs);
    _left_op = ARFactory::createOperand((*rhs)[1]);
    _right_op = ARFactory::createOperand((*rhs)[2]);
    _source_location = ARFactory::createSourceLocation(*srcloc);
  } else {
    throw parse_error("AR_FP_Op", e);
  }
}

AR_FP_Op::~AR_FP_Op() {}

void AR_FP_Op::print(std::ostream& out) {
  (*_result).print(out);
  out << " = " << getFPOpText() << "(";
  (*_left_op).print(out);
  out << ", ";
  (*_right_op).print(out);
  out << ")";
  print_sourcelocation(out);
}

std::string AR_FP_Op::getFPOpText() {
  switch (_op) {
    case fadd:
      return "fadd";
    case fsub:
      return "fsub";
    case fmul:
      return "fmul";
    case fdiv:
      return "fdiv";
    case frem:
      return "frem";
    default:
      return EM_STR;
  }
}

////
// AR_Allocate
////

AR_Allocate::AR_Allocate(AR_Node_Ref< AR_Var_Addr_Constant > local_var_ref,
                         AR_Node_Ref< AR_Type > allocated_type,
                         AR_Node_Ref< AR_Operand > array_size,
                         AR_Node_Ref< AR_Source_Location > source_loc)
    : AR_Statement(source_loc),
      _local_var_ref(local_var_ref),
      _allocated_type(allocated_type),
      _array_size(array_size) {}

AR_Allocate::AR_Allocate(index64_t parent_bblock, s_expression e)
    : AR_Statement(parent_bblock),
      _local_var_ref(AR_Null_Ref),
      _allocated_type(AR_Null_Ref),
      _array_size(AR_Null_Ref) {
  /**
   * Example s-expr:

    ($allocate
      ($dest ($cst ($localvariableref ($name ($foo.vla)) ($ty (!13)))))
      ($alloca_ty (!10))
      ($array_size ($var ($name ($foo._2)) ($ty (!7))))
      ($debug ($srcloc ($line (#3)) ($col (#5)) ($file (!11)))))
   */
  s_expression_ref local_var_ref, alloca_ty, array_size, align, srcloc;
  if (s_pattern("allocate",
                s_pattern("dest", s_pattern("cst", local_var_ref)),
                s_pattern("alloca_ty", alloca_ty),
                s_pattern("array_size", array_size),
                s_pattern("debug", srcloc)) ^
      e) {
    _local_var_ref = node_cast< AR_Var_Addr_Constant >(
        ARFactory::createConstant(*local_var_ref));
    _allocated_type = ARFactory::createType(
        (static_cast< index64_atom& >(**alloca_ty)).data());
    _array_size = ARFactory::createOperand(*array_size);
    _source_location = ARFactory::createSourceLocation(*srcloc);
  } else {
    throw parse_error("AR_Allocate", e);
  }
}

AR_Allocate::~AR_Allocate() {}

void AR_Allocate::print(std::ostream& out) {
  (*_local_var_ref).print(out);
  out << " = allocate array_size=";
  (*_array_size).print(out);
  out << " alloca_ty=";
  (*_allocated_type).print(out);
}

////
// AR_Load
////

AR_Load::AR_Load(AR_Node_Ref< AR_Internal_Variable > result,
                 AR_Node_Ref< AR_Operand > ptr,
                 AR_Node_Ref< AR_Source_Location > source_loc)
    : AR_Statement(source_loc), _result(result), _ptr(ptr) {}

AR_Load::AR_Load(index64_t parent_bblock, s_expression e)
    : AR_Statement(parent_bblock), _result(AR_Null_Ref), _ptr(AR_Null_Ref) {
  /**
   * Example s-expr: ($load ($lhs ($namedvar ($name ($tmp2)) ($ty (!4)))) ($rhs
   * ($constant ($cst (!17)) ($ty (!3)))) ($align (#))
   * ($debug ($srcloc ($line (#9)) ($file (!6)))))
   */
  s_expression_ref lhs, rhs, srcloc, align;
  if (s_pattern("load",
                s_pattern("lhs", lhs),
                s_pattern("rhs", rhs),
                align,
                s_pattern("debug", srcloc)) ^
      e) {
    _result = ARFactory::createInternalVariable(*lhs);
    _ptr = ARFactory::createOperand(*rhs);
    if ((*align).n_args() > 0) {
      s_expression align_s_expr = *align;
      s_pattern("align", align) ^ align_s_expr;
      _alignment = (static_cast< z_number_atom& >(**align)).data();
    }
    _source_location = ARFactory::createSourceLocation(*srcloc);
  } else {
    throw parse_error("AR_Load", e);
  }
}

AR_Load::~AR_Load() {}

void AR_Load::print(std::ostream& out) {
  (*_result).print(out);
  out << " = load(";
  (*_ptr).print(out);
  if (_alignment)
    out << ", align " << *_alignment;
  out << ")";
  print_sourcelocation(out);
}

////
// AR_Store
////

AR_Store::AR_Store(AR_Node_Ref< AR_Operand > ptr,
                   AR_Node_Ref< AR_Operand > value,
                   AR_Node_Ref< AR_Source_Location > source_loc)
    : AR_Statement(source_loc), _ptr(ptr), _value(value) {}

AR_Store::AR_Store(index64_t parent_bblock, s_expression e)
    : AR_Statement(parent_bblock), _ptr(AR_Null_Ref), _value(AR_Null_Ref) {
  /**
   * Example s-expr: ($store
   ($lhs ($var ($name ($d)) ($ty (!4))))
   ($rhs ($constantfp ($val (^0x1.c28f5c28f5c29p-1)) ($ty (!4))))
   ($align (#))
   ($debug ($srcloc ($line (#3)) ($file (!5)))))
   */
  s_expression_ref v, p, align, srcloc;
  if (s_pattern("store",
                s_pattern("lhs", p),
                s_pattern("rhs", v),
                align,
                s_pattern("debug", srcloc)) ^
      e) {
    _value = ARFactory::createOperand(*v);
    _ptr = ARFactory::createOperand(*p);
    if ((*align).n_args() > 0) {
      s_expression align_s_expr = *align;
      s_pattern("align", align) ^ align_s_expr;
      _alignment = (static_cast< z_number_atom& >(**align)).data();
    }
    _source_location = ARFactory::createSourceLocation(*srcloc);
  } else {
    throw parse_error("AR_Store", e);
  }
}

AR_Store::~AR_Store() {}

void AR_Store::print(std::ostream& out) {
  (*_ptr).print(out);
  out << " = store(";
  (*_value).print(out);
  if (_alignment)
    out << ", align " << *_alignment;
  out << ")";
  print_sourcelocation(out);
}

////
// AR_Extract_Element
////

AR_Extract_Element::AR_Extract_Element(
    AR_Node_Ref< AR_Internal_Variable > result,
    AR_Node_Ref< AR_Operand > value,
    AR_Node_Ref< AR_Operand > offset,
    AR_Node_Ref< AR_Source_Location > source_loc)
    : AR_Statement(source_loc),
      _result(result),
      _value(value),
      _offset(offset) {}

AR_Extract_Element::AR_Extract_Element(index64_t parent_bblock, s_expression e)
    : AR_Statement(parent_bblock),
      _result(AR_Null_Ref),
      _value(AR_Null_Ref),
      _offset(AR_Null_Ref) {
  /**
   * Example s-expression: ($extractelem
   ($var ($name ($mrv_gr)) ($ty (!14)))
   ($var ($name ($tmp)) ($ty (!13)))
   ($var ($name ($__v:4)) ($ty (!20)))
   ($debug ($srcloc ($line (#20)) ($file (!15)))))
   */
  s_expression_ref r, v, o, srcloc;
  if (s_pattern("extractelem", r, v, o, s_pattern("debug", srcloc)) ^ e) {
    _result = ARFactory::createInternalVariable(*r);
    _value = ARFactory::createOperand(*v);
    _offset = ARFactory::createOperand(*o);
    _source_location = ARFactory::createSourceLocation(*srcloc);
  } else {
    throw parse_error("AR_Extract_Element", e);
  }
}

void AR_Extract_Element::print(std::ostream& out) {
  (*_result).print(out);
  out << " = extractelem(";
  (*_value).print(out);
  out << ", ";
  (*_offset).print(out);
  out << ")";
  print_sourcelocation(out);
}

////
// AR_Insert_Element
////

AR_Insert_Element::AR_Insert_Element(
    AR_Node_Ref< AR_Internal_Variable > result,
    AR_Node_Ref< AR_Operand > value,
    AR_Node_Ref< AR_Operand > offset,
    AR_Node_Ref< AR_Operand > element,
    AR_Node_Ref< AR_Source_Location > source_loc)
    : AR_Statement(source_loc),
      _result(result),
      _value(value),
      _offset(offset),
      _element(element) {}

AR_Insert_Element::AR_Insert_Element(index64_t parent_bblock, s_expression e)
    : AR_Statement(parent_bblock),
      _result(AR_Null_Ref),
      _value(AR_Null_Ref),
      _offset(AR_Null_Ref),
      _element(AR_Null_Ref) {
  /**
   * Example s-expr: ($insertelem
   ($var ($name ($mrv4)) ($ty (!13)))
   ($var ($name ($__v:51)) ($ty (!13)))
   ($var ($name ($mrv)) ($ty (!14)))
   ($var ($name ($__v:50)) ($ty (!20)))
   ($debug ($srcloc ($line (#15)) ($file (!15)))))
   */
  s_expression_ref r, v, elem, o, srcloc;
  if (s_pattern("insertelem", r, v, elem, o, s_pattern("debug", srcloc)) ^ e) {
    _result = ARFactory::createInternalVariable(*r);
    _value = ARFactory::createOperand(*v);
    _element = ARFactory::createOperand(*elem);
    _offset = ARFactory::createOperand(*o);
    _source_location = ARFactory::createSourceLocation(*srcloc);
  } else {
    throw parse_error("AR_Insert_Element", e);
  }
}

void AR_Insert_Element::print(std::ostream& out) {
  (*_result).print(out);
  out << " = insertelem(";
  (*_value).print(out);
  out << ", ";
  (*_element).print(out);
  out << ", ";
  (*_offset).print(out);
  out << ")";
  print_sourcelocation(out);
}

////
// AR_Call
////

AR_Call::AR_Call(AR_Node_Ref< AR_Internal_Variable > return_value,
                 AR_Node_Ref< AR_Operand > called_function,
                 const std::vector< AR_Node_Ref< AR_Operand > >& args,
                 bool is_inline_asm,
                 AR_Node_Ref< AR_Source_Location > source_loc)
    : AR_Statement(source_loc),
      _return_value(return_value),
      _called_function(called_function),
      _args(args),
      _is_inline_asm(is_inline_asm) {}

AR_Call::AR_Call(index64_t parent_bblock, s_expression e)
    : AR_Statement(parent_bblock),
      _return_value(AR_Null_Ref),
      _called_function(AR_Null_Ref),
      _is_inline_asm(false) {
  /**
   * Example s-expr:
     ($call
       ($ret ($var ($name ($__v:52)) ($ty (!33))))
       ($callee ($cst ($constantfunctionptr ($name ($__cxa_call_unexpected))
   ($ty (!46)))))
       ($args ($a ($var ($name ($_Z8divisionii._12)) ($ty (!12)))))
       ($inlineAsm ($false))
       ($debug ($srcloc ($line (#10)) ($file (!41)))))
   */
  s_expression_ref ret, callee, args_sexpr_ref, is_inline_asm, srcloc;
  if (s_pattern("call",
                s_pattern("ret", ret),
                s_pattern("callee", callee),
                args_sexpr_ref,
                s_pattern("inlineAsm", is_inline_asm),
                s_pattern("debug", srcloc)) ^
      e) {
    // Get return value
    _return_value = ARFactory::createInternalVariable(*ret);

    // Get called function
    _called_function = ARFactory::createOperand(*callee);

    for (size_t i = 1; i <= (*args_sexpr_ref).n_args(); i++) {
      s_expression_ref a;
      if (s_pattern("a", a) ^ (*args_sexpr_ref)[i]) {
        AR_Node_Ref< AR_Operand > arg_ref(ARFactory::createOperand(*a));
        _args.push_back(arg_ref);
      } else {
        throw parse_error("AR_Call, create argument operand", *a);
      }
    }
    std::string _is_inline_asm =
        (static_cast< string_atom& >(**is_inline_asm)).data();
    _source_location = ARFactory::createSourceLocation(*srcloc);
  } else {
    throw parse_error("AR_Call", e);
  }
}

AR_Call::~AR_Call() {}

void AR_Call::print(std::ostream& out) {
  (*_return_value).print(out);
  out << " = call ";
  (*_called_function).print(out);

  std::vector< AR_Node_Ref< AR_Operand > >::iterator i = _args.begin(),
                                                     next = _args.begin();
  next++;
  out << "(";
  for (; i != _args.end(); i++, next++) {
    (**i).print(out);
    if (next != _args.end())
      out << ", ";
  }
  out << ")";

  out << "isInlineAsm=" << (_is_inline_asm ? "true" : "false");
  print_sourcelocation(out);
}

////
// AR_NOP
////

AR_NOP::AR_NOP(AR_Node_Ref< AR_Source_Location > source_loc)
    : AR_Statement(source_loc) {}

AR_NOP::AR_NOP(index64_t parent_bblock) : AR_Statement(parent_bblock) {}

AR_NOP::~AR_NOP() {}

void AR_NOP::print(std::ostream& out) {
  out << "no-op";
  print_sourcelocation(out);
}

////
// AR_Invoke
////

AR_Invoke::AR_Invoke(AR_Node_Ref< AR_Call > call,
                     AR_Node_Ref< AR_Basic_Block > normal,
                     AR_Node_Ref< AR_Basic_Block > exception,
                     AR_Node_Ref< AR_Source_Location > source_loc)
    : AR_Statement(source_loc),
      _call(call),
      _normal(normal.getUID()),
      _exception(exception.getUID()) {}

AR_Invoke::AR_Invoke(index64_t parent_bblock, s_expression e)
    : AR_Statement(parent_bblock),
      _call(AR_Null_Ref),
      _normal(0),
      _exception(0) {
  /**
   * Example s-expr:
     ($invoke
       ($call ($ret ($var ($name ($__v:49)) ($ty (!33))))
              ($callee ($cst ($constantfunctionptr ($name ($__cxa_throw)) ($ty
   (!44)))))
              ($args
                ($a ($var ($name ($_Z8divisionii.exception)) ($ty (!14))))
                ($a ($var ($name ($_Z8divisionii._2)) ($ty (!14))))
                ($a ($cst ($constantpointernull ($ty (!14))))))
       ($debug ($srcloc ($line (#8)) ($file (!41)))))
       ($br ($normal ($unreachable)) ($exception ($lpad))) ($debug ($srcloc
   ($line (#8)) ($file (!41)))))
   */
  s_expression_ref c, normal, except, srcloc;
  if (s_pattern("invoke",
                c,
                s_pattern("br",
                          s_pattern("normal", normal),
                          s_pattern("exception", except)),
                s_pattern("debug", srcloc)) ^
      e) {
    _call = AR_Call::create(parent_bblock, *c);
    _normal_bb_name = (static_cast< string_atom& >(**normal)).data();
    _except_bb_name = (static_cast< string_atom& >(**except)).data();
    _source_location = ARFactory::createSourceLocation(*srcloc);
    ARModel::Instance()->add_listener(NODESCREATED, getUID());
  } else {
    throw parse_error("AR_Invoke", e);
  }
}

void AR_Invoke::nodesCreated() {
  AR_Node_Ref< AR_Basic_Block > parent_bb = getContainingBasicBlock();
  AR_Node_Ref< AR_Code > parent_code = (*parent_bb).getContainingCode();
  _normal = (*parent_code).getBasicBlockByNameId(_normal_bb_name).getUID();
  _exception = (*parent_code).getBasicBlockByNameId(_except_bb_name).getUID();
  _normal_bb_name.clear();
  _except_bb_name.clear();
}

void AR_Invoke::print(std::ostream& out) {
  out << "invoke: !" << getUID() << " ";
  (*_call).print(out);
  out << ", ";
  out << "nomral=" << getNormal()->getNameId() << " !" << _normal
      << ", exception=" << getException()->getNameId() << " !" << _exception;
  print_sourcelocation(out);
}

AR_Invoke::~AR_Invoke() {}

////
// AR_Abstract_Memory
////

AR_Abstract_Memory::AR_Abstract_Memory(
    AR_Node_Ref< AR_Operand > ptr,
    AR_Node_Ref< AR_Operand > len,
    AR_Node_Ref< AR_Source_Location > source_loc)
    : AR_Statement(source_loc), _ptr(ptr), _len(len) {}

AR_Abstract_Memory::AR_Abstract_Memory(index64_t parent_bblock, s_expression e)
    : AR_Statement(parent_bblock), _ptr(AR_Null_Ref), _len(AR_Null_Ref) {
  /**
   * Example s-expr: ($abstractmemory
   ($ptr ($cst ($globalvariableref ($name
   ($_ZTVN10__cxxabiv117__class_type_infoE)) ($gv (!4)) ($ty (!21)))))
   ($len ($cst ($constantint ($val (#0)) ($ty (!20)))))
   ($debug ($srcloc ($line (#-1)) ($file (!13)))))
   */
  s_expression_ref p, l, srcloc;
  if (s_pattern("abstractmemory",
                s_pattern("ptr", p),
                s_pattern("len", l),
                s_pattern("debug", srcloc)) ^
      e) {
    _ptr = ARFactory::createOperand(*p);
    _len = ARFactory::createOperand(*l);
    _source_location = ARFactory::createSourceLocation(*srcloc);
  } else {
    throw parse_error("AR_Abstract_Memory", e);
  }
}

void AR_Abstract_Memory::print(std::ostream& out) {
  out << "abstractmem (";
  (*_ptr).print(out);
  out << ", ";
  (*_len).print(out);
  out << ")";
  print_sourcelocation(out);
}

////
// AR_Abstract_Variable
////

AR_Abstract_Variable::AR_Abstract_Variable(
    AR_Node_Ref< AR_Internal_Variable > variable,
    AR_Node_Ref< AR_Source_Location > source_loc)
    : AR_Statement(source_loc), _variable(variable) {}

AR_Abstract_Variable::AR_Abstract_Variable(index64_t parent_bblock,
                                           s_expression e)
    : AR_Statement(parent_bblock), _variable(AR_Null_Ref) {
  /**
   * Example s-expr: ($abstractvariable ($var ($name ($__v:63)) ($ty (!24)))
   * ($debug ($srcloc ($line (#31)) ($file (!27)))))
   */
  s_expression_ref v, srcloc;
  if (s_pattern("abstractvariable", v, s_pattern("debug", srcloc)) ^ e) {
    _variable = ARFactory::createInternalVariable(*v);
    _source_location = ARFactory::createSourceLocation(*srcloc);
  } else {
    throw parse_error("AR_Abstract_Variable", e);
  }
}

void AR_Abstract_Variable::print(std::ostream& out) {
  out << "abstractvar ";
  (*_variable).print(out);
  print_sourcelocation(out);
}

////
// AR_Unreachable
////

AR_Unreachable::AR_Unreachable(AR_Node_Ref< AR_Source_Location > source_loc)
    : AR_Statement(source_loc) {}

AR_Unreachable::AR_Unreachable(index64_t parent_bblock, s_expression e)
    : AR_Statement(parent_bblock) {
  /**
   * Example s-expr: ($unreachable ($debug ($srcloc ($line (#48)) ($file
   * (!27)))))
   */
  s_expression_ref srcloc;
  if (s_pattern("unreachable", s_pattern("debug", srcloc)) ^ e) {
    _source_location = ARFactory::createSourceLocation(*srcloc);
  } else {
    throw parse_error("AR_Unreachable", e);
  }
}

AR_Unreachable::~AR_Unreachable() {}

void AR_Unreachable::print(std::ostream& out) {
  out << "unreachable";
  print_sourcelocation(out);
}

////
// AR_Landing_Pad
////

AR_Landing_Pad::AR_Landing_Pad(AR_Node_Ref< AR_Internal_Variable > exception,
                               AR_Node_Ref< AR_Source_Location > source_loc)
    : AR_Statement(source_loc), _exception(exception) {}

AR_Landing_Pad::AR_Landing_Pad(index64_t parent_bblock, s_expression e)
    : AR_Statement(parent_bblock), _exception(AR_Null_Ref) {
  /**
   * Example s-expr:
     ($landingpad
        ($exception
           ($var ($name ($_Z8divisionii._lpad)) ($ty (!52))))
        ($debug ($srcloc ($line (#11)) ($file (!48)))))
   */

  s_expression_ref v, srcloc;
  if (s_pattern("landingpad",
                s_pattern("exception", v),
                s_pattern("debug", srcloc)) ^
      e) {
    _exception = ARFactory::createInternalVariable(*v);
    _source_location = ARFactory::createSourceLocation(*srcloc);
  } else {
    throw parse_error("AR_Landing_Pad", e);
  }
}

void AR_Landing_Pad::print(std::ostream& out) {
  out << "landingpad ex=";
  (*_exception).print(out);
  print_sourcelocation(out);
}

////
// AR_Resume
////

AR_Resume::AR_Resume(AR_Node_Ref< AR_Internal_Variable > exception,
                     AR_Node_Ref< AR_Source_Location > source_loc)
    : AR_Statement(source_loc), _exception(exception) {}

AR_Resume::AR_Resume(index64_t parent_bblock, s_expression e)
    : AR_Statement(parent_bblock), _exception(AR_Null_Ref) {
  /**
   * Example s-expr:
     ($resume
       ($exception
          ($var ($name ($__v:173)) ($ty (!40))))
       ($debug ($srcloc ($line (#22)) ($file (!48)))))
   */

  s_expression_ref v, srcloc;
  if (s_pattern("resume",
                s_pattern("exception", v),
                s_pattern("debug", srcloc)) ^
      e) {
    _exception = ARFactory::createInternalVariable(*v);
    _source_location = ARFactory::createSourceLocation(*srcloc);
  } else {
    throw parse_error("AR_Resume", e);
  }
}

void AR_Resume::print(std::ostream& out) {
  out << "resume ex=";
  (*_exception).print(out);
  print_sourcelocation(out);
}

////
// AR_MemCpy
////

AR_MemCpy::AR_MemCpy(AR_Node_Ref< AR_Internal_Variable > result,
                     AR_Node_Ref< AR_Operand > tgt_pointer,
                     AR_Node_Ref< AR_Operand > src_pointer,
                     AR_Node_Ref< AR_Operand > len,
                     AR_Node_Ref< AR_Source_Location > source_loc)
    : AR_Statement(source_loc),
      _result(result),
      _tgt_pointer(tgt_pointer),
      _src_pointer(src_pointer),
      _len(len) {}

AR_MemCpy::AR_MemCpy(index64_t parent_bblock, s_expression e)
    : AR_Statement(parent_bblock),
      _result(AR_Null_Ref),
      _tgt_pointer(AR_Null_Ref),
      _src_pointer(AR_Null_Ref),
      _len(AR_Null_Ref) {
  /**
   * Example s-expr:

   ($memcpy
   ($ret ())
   ($dest ())
   ($src ())
   ($len ())
   ($align (#)
   ($isvolatile ($constant ($cst (!17)) ($ty (!18))))
   ($debug ($srcloc ($line (#7)) ($file (!6)))))
   */
  s_expression_ref ret, d, l, src, align, srcloc;
  if (s_pattern("memcpy",
                s_pattern("ret", ret),
                s_pattern("dest", d),
                s_pattern("src", src),
                s_pattern("len", l),
                s_pattern("align", align),
                s_pattern(),
                s_pattern("debug", srcloc)) ^
      e) {
    _result = ARFactory::createInternalVariable(*ret);
    _tgt_pointer = ARFactory::createOperand(*d);
    _src_pointer = ARFactory::createOperand(*src);
    _len = ARFactory::createOperand(*l);
    if ((*align).n_args() > 0) {
      _alignment = (static_cast< z_number_atom& >(**align)).data();
    }
    _source_location = ARFactory::createSourceLocation(*srcloc);
  } else {
    throw parse_error("AR_Memcpy", e);
  }
}

AR_MemCpy::~AR_MemCpy() {}

void AR_MemCpy::print(std::ostream& out) {
  (*_result).print(out);
  out << " = memcpy(";
  (*_tgt_pointer).print(out);
  out << ", ";
  (*_src_pointer).print(out);
  out << ", ";
  (*_len).print(out);
  if (_alignment)
    out << ", align " << *_alignment;
  out << ")";
  print_sourcelocation(out);
}

////
// AR_MemMove
////

AR_MemMove::AR_MemMove(AR_Node_Ref< AR_Internal_Variable > result,
                       AR_Node_Ref< AR_Operand > tgt_pointer,
                       AR_Node_Ref< AR_Operand > src_pointer,
                       AR_Node_Ref< AR_Operand > len,
                       AR_Node_Ref< AR_Source_Location > source_loc)
    : AR_Statement(source_loc),
      _result(result),
      _tgt_pointer(tgt_pointer),
      _src_pointer(src_pointer),
      _len(len) {}

AR_MemMove::AR_MemMove(index64_t parent_bblock, s_expression e)
    : AR_Statement(parent_bblock),
      _result(AR_Null_Ref),
      _tgt_pointer(AR_Null_Ref),
      _src_pointer(AR_Null_Ref),
      _len(AR_Null_Ref) {
  /**
   * Example s-expr:

   ($memmove
   ($ret ())
   ($dest ())
   ($src ())
   ($len ())
   ($align (#)
   ($isvolatile ($constant ($cst (!17)) ($ty (!18))))
   ($debug ($srcloc ($line (#7)) ($file (!6)))))
   */
  s_expression_ref ret, d, l, src, align, srcloc;
  if (s_pattern("memmove",
                s_pattern("ret", ret),
                s_pattern("dest", d),
                s_pattern("src", src),
                s_pattern("len", l),
                s_pattern("align", align),
                s_pattern(),
                s_pattern("debug", srcloc)) ^
      e) {
    _result = ARFactory::createInternalVariable(*ret);
    _tgt_pointer = ARFactory::createOperand(*d);
    _src_pointer = ARFactory::createOperand(*src);
    _len = ARFactory::createOperand(*l);
    if ((*align).n_args() > 0) {
      _alignment = (static_cast< z_number_atom& >(**align)).data();
    }
    _source_location = ARFactory::createSourceLocation(*srcloc);
  } else {
    throw parse_error("AR_Memmove", e);
  }
}

AR_MemMove::~AR_MemMove() {}

void AR_MemMove::print(std::ostream& out) {
  (*_result).print(out);
  out << " = memmove(";
  (*_tgt_pointer).print(out);
  out << ", ";
  (*_src_pointer).print(out);
  out << ", ";
  (*_len).print(out);
  if (_alignment)
    out << ", align " << *_alignment;
  out << ")";
}

////
// AR_MemSet
////

AR_MemSet::AR_MemSet(AR_Node_Ref< AR_Internal_Variable > result,
                     AR_Node_Ref< AR_Operand > ptr,
                     AR_Node_Ref< AR_Operand > value,
                     AR_Node_Ref< AR_Operand > len,
                     AR_Node_Ref< AR_Source_Location > source_loc)
    : AR_Statement(source_loc),
      _result(result),
      _ptr(ptr),
      _value(value),
      _len(len) {}

AR_MemSet::AR_MemSet(index64_t parent_bblock, s_expression e)
    : AR_Statement(parent_bblock),
      _result(AR_Null_Ref),
      _ptr(AR_Null_Ref),
      _value(AR_Null_Ref),
      _len(AR_Null_Ref) {
  /**
   * Example s-expr:

   ($memset
   ($ret ($var ($name ($\5C5Fv)) ($ty (!12))))
   ($dest ($var ($name ($\5C5F\5C5Fv)) ($ty (!9))))
   ($val ($constant ($cst (!13)) ($ty (!10))))
   ($len ($constant ($cst (!14)) ($ty (!15))))
   ($align (#)
   ($isvolatile ($constant ($cst (!17)) ($ty (!18))))
   ($debug ($srcloc ($line (#7)) ($file (!6)))))
   */
  s_expression_ref ret, d, l, a, v, align, srcloc;
  if (s_pattern("memset",
                s_pattern("ret", ret),
                s_pattern("dest", d),
                s_pattern("val", v),
                s_pattern("len", l),
                s_pattern("align", align),
                s_pattern(),
                s_pattern("debug", srcloc)) ^
      e) {
    _result = ARFactory::createInternalVariable(*ret);
    _ptr = ARFactory::createOperand(*d);
    _value = ARFactory::createOperand(*v);
    _len = ARFactory::createOperand(*l);
    if ((*align).n_args() > 0) {
      _alignment = (static_cast< z_number_atom& >(**align)).data();
    }
    _source_location = ARFactory::createSourceLocation(*srcloc);
  } else {
    throw parse_error("AR_Memset", e);
  }
}

AR_MemSet::~AR_MemSet() {}

void AR_MemSet::print(std::ostream& out) {
  (*_result).print(out);
  out << " = memset(";
  (*_ptr).print(out);
  out << ", ";
  (*_value).print(out);
  out << ", ";
  (*_len).print(out);
  if (_alignment)
    out << ", align " << *_alignment;
  out << ")";
  print_sourcelocation(out);
}

////
// AR_VA_Start
////

AR_VA_Start::AR_VA_Start(AR_Node_Ref< AR_Operand > va_list_ptr,
                         AR_Node_Ref< AR_Source_Location > source_loc)
    : AR_Statement(source_loc), _va_list_ptr(va_list_ptr) {}

AR_VA_Start::AR_VA_Start(index64_t parent_bblock, s_expression e)
    : AR_Statement(parent_bblock), _va_list_ptr(AR_Null_Ref) {
  /**
   * Example s-expr: ($va_start ($var ($name ($vl12)) ($ty (!6))) ($debug
   * ($srcloc ($line (#14)) ($file (!14)))))
   */
  s_expression_ref ptr, srcloc;
  if (s_pattern("va_start", ptr, s_pattern("debug", srcloc)) ^ e) {
    _va_list_ptr = ARFactory::createOperand(*ptr);
    _source_location = ARFactory::createSourceLocation(*srcloc);
  } else {
    throw parse_error("AR_VA_Start", e);
  }
}

void AR_VA_Start::print(std::ostream& out) {
  out << "va_start(";
  (*_va_list_ptr).print(out);
  out << ")";
  print_sourcelocation(out);
}

////
// AR_VA_End
////

AR_VA_End::AR_VA_End(AR_Node_Ref< AR_Operand > va_list_ptr,
                     AR_Node_Ref< AR_Source_Location > source_loc)
    : AR_Statement(source_loc), _va_list_ptr(va_list_ptr) {}

AR_VA_End::AR_VA_End(index64_t parent_bblock, s_expression e)
    : AR_Statement(parent_bblock), _va_list_ptr(AR_Null_Ref) {
  /**
   * Example s-expr: ($va_end ($var ($name ($vl_count1213)) ($ty (!6))) ($debug
   * ($srcloc ($line (#22)) ($file (!14)))))
   */
  s_expression_ref ptr, srcloc;
  if (s_pattern("va_end", ptr, s_pattern("debug", srcloc)) ^ e) {
    _va_list_ptr = ARFactory::createOperand(*ptr);
    _source_location = ARFactory::createSourceLocation(*srcloc);
  } else {
    throw parse_error("AR_VA_End", e);
  }
}

void AR_VA_End::print(std::ostream& out) {
  out << "va_end(";
  (*_va_list_ptr).print(out);
  out << ")";
  print_sourcelocation(out);
}

////
// AR_VA_Copy
////

AR_VA_Copy::AR_VA_Copy(AR_Node_Ref< AR_Operand > dest_va_list_ptr,
                       AR_Node_Ref< AR_Operand > src_va_list_ptr,
                       AR_Node_Ref< AR_Source_Location > source_loc)
    : AR_Statement(source_loc),
      _dest_va_list_ptr(dest_va_list_ptr),
      _src_va_list_ptr(src_va_list_ptr) {}

AR_VA_Copy::AR_VA_Copy(index64_t parent_bblock, s_expression e)
    : AR_Statement(parent_bblock),
      _dest_va_list_ptr(AR_Null_Ref),
      _src_va_list_ptr(AR_Null_Ref) {
  /**
   * Example s-expr: ($va_copy ($dest ($var ($name ($vl_count35)) ($ty (!6))))
   * ($src ($var ($name ($vl46)) ($ty (!6)))) ($debug ($srcloc ($line (#17))
   * ($file (!14)))))
   */
  s_expression_ref dest, src, srcloc;
  if (s_pattern("va_copy",
                s_pattern("dest", dest),
                s_pattern("src", src),
                s_pattern("debug", srcloc)) ^
      e) {
    _dest_va_list_ptr = ARFactory::createOperand(*dest);
    _src_va_list_ptr = ARFactory::createOperand(*src);
    _source_location = ARFactory::createSourceLocation(*srcloc);
  } else {
    throw parse_error("AR_VA_Copy", e);
  }
}

void AR_VA_Copy::print(std::ostream& out) {
  out << "va_copy(";
  (*_dest_va_list_ptr).print(out);
  out << ", ";
  (*_src_va_list_ptr).print(out);
  out << ")";
  print_sourcelocation(out);
}

////
// AR_VA_Arg
////

AR_VA_Arg::AR_VA_Arg(AR_Node_Ref< AR_Internal_Variable > result,
                     AR_Node_Ref< AR_Operand > va_list_ptr,
                     AR_Node_Ref< AR_Source_Location > source_loc)
    : AR_Statement(source_loc), _result(result), _va_list_ptr(va_list_ptr) {}

AR_VA_Arg::AR_VA_Arg(index64_t parent_bblock, s_expression e)
    : AR_Statement(parent_bblock),
      _result(AR_Null_Ref),
      _va_list_ptr(AR_Null_Ref) {
  /**
   * Example s-expr: ($va_arg ($ret) ($ptr) ($debug))
   */
  s_expression_ref ret, ptr, srcloc;
  if (s_pattern("va_arg",
                s_pattern("ret", ret),
                s_pattern("ptr", ptr),
                s_pattern("debug", srcloc)) ^
      e) {
    _result = ARFactory::createInternalVariable(*ret);
    _va_list_ptr = ARFactory::createOperand(*ptr);
    _source_location = ARFactory::createSourceLocation(*srcloc);
  } else {
    throw parse_error("AR_VA_Arg", e);
  }
}

void AR_VA_Arg::print(std::ostream& out) {
  (*_result).print(std::cerr);
  out << " = va_arg(";
  (*_va_list_ptr).print(out);
  out << ")";
  print_sourcelocation(out);
}

////
// AR_Assignment
////

AR_Assignment::AR_Assignment(AR_Node_Ref< AR_Internal_Variable > left_op,
                             AR_Node_Ref< AR_Operand > right_op,
                             AR_Node_Ref< AR_Source_Location > source_loc)
    : AR_Statement(source_loc), _left_op(left_op), _right_op(right_op) {}

AR_Assignment::AR_Assignment(index64_t parent_bblock, s_expression e)
    : AR_Statement(parent_bblock),
      _left_op(AR_Null_Ref),
      _right_op(AR_Null_Ref) {
  /**
   * Example s-expr: ($assign ($var ($name ($__v:9)) ($ty (!16))) ($var ($name
   * ($__v:7)) ($ty (!25))) ($debug ($srcloc)))
   */
  s_expression_ref lhs, rhs, srcloc;
  if (s_pattern("assign",
                s_pattern("lhs", lhs),
                s_pattern("rhs", rhs),
                s_pattern("debug", srcloc)) ^
      e) {
    _left_op = ARFactory::createInternalVariable(*lhs);
    assert(_left_op.getUID() > 0);
    _right_op = ARFactory::createOperand(*rhs);
    _source_location = ARFactory::createSourceLocation(*srcloc);
  } else {
    throw parse_error("AR_Assignment", e);
  }
}

void AR_Assignment::print(std::ostream& out) {
  (*_left_op).print(out);
  out << " = ";
  (*_right_op).print(out);
  print_sourcelocation(out);
}

AR_Assignment::~AR_Assignment() {}

////
// AR_Pointer_Shift
////

AR_Pointer_Shift::AR_Pointer_Shift(AR_Node_Ref< AR_Internal_Variable > result,
                                   AR_Node_Ref< AR_Operand > ptr,
                                   AR_Node_Ref< AR_Operand > offset,
                                   AR_Node_Ref< AR_Source_Location > source_loc)
    : AR_Statement(source_loc), _result(result), _ptr(ptr), _offset(offset) {}

AR_Pointer_Shift::AR_Pointer_Shift(index64_t parent_bblock, s_expression e)
    : AR_Statement(parent_bblock),
      _result(AR_Null_Ref),
      _ptr(AR_Null_Ref),
      _offset(AR_Null_Ref) {
  /**
   * Example s-expr: ($ptrshift
   ($lhs ($var ($name ($__v:1)) ($ty (!19))))
   ($rhs ($cst ($globalvariableref ($name ($_ZTI1A)) ($gv (!3)) ($ty (!14))))
   ($cst ($constantint ($val (#0)) ($ty (!20)))))
   ($debug ($srcloc ($line (#-1)) ($file (!13)))))
   */
  s_expression_ref lhs, rhs, srcloc;
  if (s_pattern("ptrshift",
                s_pattern("lhs", lhs),
                rhs,
                s_pattern("debug", srcloc)) ^
      e) {
    _result = ARFactory::createInternalVariable(*lhs);
    _ptr = ARFactory::createOperand((*rhs)[1]);
    _offset = ARFactory::createOperand((*rhs)[2]);
    _source_location = ARFactory::createSourceLocation(*srcloc);
  } else {
    throw parse_error("AR_Pointer_Shift", e);
  }
}

AR_Pointer_Shift::~AR_Pointer_Shift() {}

void AR_Pointer_Shift::print(std::ostream& out) {
  (*_result).print(out);
  out << " = ptrshift(";
  (*_ptr).print(out);
  out << ", ";
  (*_offset).print(out);
  out << ")";
  print_sourcelocation(out);
}

////
// AR_Return_Value
////

AR_Return_Value::AR_Return_Value(AR_Node_Ref< AR_Operand > value,
                                 AR_Node_Ref< AR_Source_Location > source_loc)
    : AR_Statement(source_loc), _value(value) {}

AR_Return_Value::AR_Return_Value(index64_t parent_bblock, s_expression e)
    : AR_Statement(parent_bblock), _value(AR_Null_Ref) {
  /**
   * Example s-expr: ($ret ($rhs ($cst ($constantint ($val (#0)) ($ty (!2)))))
   * ($debug ($srcloc ($line (#9)) ($file (!8)))))
   */
  s_expression_ref rhs, srcloc;
  if (s_pattern("ret", rhs, s_pattern("debug", srcloc)) ^ e) {
    s_expression_ref val;
    if (s_pattern("rhs", val) ^ *rhs) {
      _value = ARFactory::createOperand(*val);
    } else {
      // No return value
    }

    _source_location = ARFactory::createSourceLocation(*srcloc);
  } else {
    throw parse_error("AR_Return_Value", e);
  }
}

AR_Return_Value::~AR_Return_Value() {}

void AR_Return_Value::print(std::ostream& out) {
  out << "ret ";
  if (_value.getUID() > 0) {
    (*_value).print(out);
  } else {
    out << "none";
  }
  print_sourcelocation(out);
}

////
// AR_Basic_Block
////

AR_Basic_Block::AR_Basic_Block(const std::string& name_id)
    : _name_id(name_id), _containing_code(0) {}

AR_Basic_Block::AR_Basic_Block(index64_t parent_code, s_expression e)
    : _containing_code(parent_code) {
  /**
   * Example s-expr: ($basicblock ($name ()) ($instructions () ...) ... )
   */
  s_expression_ref n, instructions;
  if (s_pattern("basicblock", s_pattern("name", n), instructions) ^ e) {
    _name_id = (static_cast< string_atom& >(**n)).data();

    // Parse instructions
    for (size_t i = 1; i <= instructions->n_args(); i++) {
      s_expression inst = (*instructions)[i];
      std::vector< AR_Node_Ref< AR_Statement > > stmts =
          ARFactory::createStatements(getUID(), inst);

      std::vector< AR_Node_Ref< AR_Statement > >::iterator p = stmts.begin();
      for (; p != stmts.end(); p++) {
        _body.push_back(*p);
      }
    }
  } else {
    throw parse_error("AR_Basic_Block", e);
  }
}

AR_Basic_Block::~AR_Basic_Block() {}

void AR_Basic_Block::insertFront(AR_Node_Ref< AR_Statement > stmt) {
  _body.insert(_body.begin(), stmt);
  stmt->setContainingBasicBlock(getUID());
}

void AR_Basic_Block::pushBack(AR_Node_Ref< AR_Statement > stmt) {
  _body.push_back(stmt);
  stmt->setContainingBasicBlock(getUID());
}

void AR_Basic_Block::pushBack(
    const std::vector< AR_Node_Ref< AR_Statement > >& stmts) {
  for (auto it = stmts.begin(); it != stmts.end(); it++) {
    pushBack(*it);
  }
}

void AR_Basic_Block::popBack() {
  AR_Node_Ref< AR_Statement > stmt = _body.back();
  _body.pop_back();
  stmt->setContainingBasicBlock(0);
}

void AR_Basic_Block::replace(AR_Node_Ref< AR_Statement > old_stmt,
                             AR_Node_Ref< AR_Statement > new_stmt) {
  std::replace(_body.begin(), _body.end(), old_stmt, new_stmt);
  old_stmt->setContainingBasicBlock(0);
  new_stmt->setContainingBasicBlock(getUID());
}

bool AR_Basic_Block::isNextBlock(AR_Node_Ref< AR_Basic_Block > bblock) const {
  for (auto it = _next_blocks.begin(); it != _next_blocks.end(); it++) {
    if (*it == bblock.getUID()) {
      return true;
    }
  }
  return false;
}

bool AR_Basic_Block::isPreviousBlock(
    AR_Node_Ref< AR_Basic_Block > bblock) const {
  for (auto it = _prev_blocks.begin(); it != _prev_blocks.end(); it++) {
    if (*it == bblock.getUID()) {
      return true;
    }
  }
  return false;
}

void AR_Basic_Block::remove(const AR_Node_Ref< AR_Statement >& stmt) {
  _body.erase(std::remove(_body.begin(), _body.end(), stmt));
  stmt->setContainingBasicBlock(0);
}

void AR_Basic_Block::addNextBasicBlock(AR_Node_Ref< AR_Basic_Block > bblock) {
  _next_blocks.push_back(bblock.getUID());
  bblock->_prev_blocks.push_back(getUID());
}

void AR_Basic_Block::removeNextBasicBlock(
    AR_Node_Ref< AR_Basic_Block > bblock) {
  auto it =
      std::find(_next_blocks.begin(), _next_blocks.end(), bblock.getUID());
  assert(it != _next_blocks.end());
  _next_blocks.erase(it);
  it = std::find(bblock->_prev_blocks.begin(),
                 bblock->_prev_blocks.end(),
                 getUID());
  assert(it != bblock->_prev_blocks.end());
  bblock->_prev_blocks.erase(it);
}

void AR_Basic_Block::addPreviousBasicBlock(
    AR_Node_Ref< AR_Basic_Block > bblock) {
  _prev_blocks.push_back(bblock.getUID());
  bblock->_next_blocks.push_back(getUID());
}

void AR_Basic_Block::removePreviousBasicBlock(
    AR_Node_Ref< AR_Basic_Block > bblock) {
  auto it =
      std::find(_prev_blocks.begin(), _prev_blocks.end(), bblock.getUID());
  assert(it != _prev_blocks.end());
  _prev_blocks.erase(it);
  it = std::find(bblock->_next_blocks.begin(),
                 bblock->_next_blocks.end(),
                 getUID());
  assert(it != bblock->_next_blocks.end());
  bblock->_next_blocks.erase(it);
}

void AR_Basic_Block::print(std::ostream& out) {
  out << "def bb!" << getUID() << " " << _name_id << std::endl;
  std::vector< AR_Node_Ref< AR_Statement > >::iterator i = _body.begin();
  for (; i != _body.end(); i++) {
    (**i).print(out);
    out << std::endl;
  }
}

////
// AR_Code
////

AR_Code::AR_Code()
    : _entry_block(0),
      _exit_block(0),
      _unreachable_block(0),
      _ehresume_block(0),
      _parent_function(0) {}

AR_Code::AR_Code(AR_Node_Ref< AR_Function > parent_function)
    : _entry_block(0),
      _exit_block(0),
      _unreachable_block(0),
      _ehresume_block(0),
      _parent_function(parent_function.getUID()) {}

AR_Code::AR_Code(index64_t parent_function, s_expression e)
    : _entry_block(0),
      _exit_block(0),
      _unreachable_block(0),
      _ehresume_block(0),
      _parent_function(parent_function) {
  /**
   * Example s-expr: ($code ($entry ($init)) ($exit ($init)) ($basicblocks
   * ($basicblock) ...) ($trans ($edge () ()) ...))
   */
  s_expression_ref en, ex, ur, ehr, bbs, t;
  if (s_pattern("code", s_pattern("entry", en), ex, ur, ehr, bbs, t) ^ e) {
    ARModel::Instance()->setCurrentBuildingScope(
        AR_Node_Ref< AR_Code >(getUID()));

    for (size_t i = 1; i <= (*bbs).n_args(); i++) {
      s_expression bb = (*bbs)[i];
      AR_Node_Ref< AR_Basic_Block > bblock_ref =
          ARFactory::createBasicBlock(getUID(), bb);
      _blocks.push_back(bblock_ref);
    }
    connect_basic_blocks(*t);

    _entry_block =
        getBasicBlockByNameId((static_cast< string_atom& >(**en)).data())
            .getUID();
    _exit_block = (*ex).n_args() == 0
                      ? 0
                      : getBasicBlockByNameId(
                            (static_cast< string_atom& >(*(*ex)[1])).data())
                            .getUID();
    _unreachable_block =
        (*ur).n_args() == 0
            ? 0
            : getBasicBlockByNameId(
                  (static_cast< string_atom& >(*(*ur)[1])).data())
                  .getUID();
    _ehresume_block =
        (*ehr).n_args() == 0
            ? 0
            : getBasicBlockByNameId(
                  (static_cast< string_atom& >(*(*ehr)[1])).data())
                  .getUID();
  } else {
    throw parse_error("AR_Code", e);
  }
}

void AR_Code::addBasicBlock(AR_Node_Ref< AR_Basic_Block > bblock) {
  _blocks.push_back(bblock);
  bblock->setContainingCode(getUID());
}

void AR_Code::removeBasicBlock(AR_Node_Ref< AR_Basic_Block > bblock) {
  assert(std::find(_blocks.begin(), _blocks.end(), bblock) != _blocks.end());
  if (_entry_block == bblock.getUID()) {
    _entry_block = 0;
  }
  if (_exit_block == bblock.getUID()) {
    _exit_block = 0;
  }
  if (_unreachable_block == bblock.getUID()) {
    _unreachable_block = 0;
  }
  if (_ehresume_block == bblock.getUID()) {
    _ehresume_block = 0;
  }
  _blocks.erase(std::find(_blocks.begin(), _blocks.end(), bblock));
  bblock->setContainingCode(0);
}

AR_Node_Ref< AR_Internal_Variable > AR_Code::getInternalVariable(
    const std::string& name) {
  for (auto it = _internal_variables.begin(); it != _internal_variables.end();
       it++) {
    if ((*it)->getName() == name) {
      return *it;
    }
  }
  return AR_Null_Ref;
}

void AR_Code::addInternalVariable(
    AR_Node_Ref< AR_Internal_Variable > internal_var) {
  _internal_variables.push_back(internal_var);
}

AR_Node_Ref< AR_Basic_Block > AR_Code::getBasicBlockByNameId(
    const std::string& name) const {
  if (name.empty())
    return AR_Null_Ref;
  for (auto it = _blocks.begin(); it != _blocks.end(); it++) {
    if ((*it)->getNameId() == name) {
      return *it;
    }
  }
  return AR_Null_Ref;
}

void AR_Code::connect_basic_blocks(s_expression e) {
  /**
   * Example s-expr: ($trans ($edge ($entry) ($return)))
   */
  if (s_pattern("trans") ^ e) {
    // temporary map: name => basic block
    std::unordered_map< std::string, AR_Node_Ref< AR_Basic_Block > >
        name_to_bblock;
    for (auto it = _blocks.begin(); it != _blocks.end(); it++) {
      name_to_bblock[(*it)->getNameId()] = *it;
    }

    // create all edges
    for (size_t i = 1; i <= e.n_args(); i++) {
      s_expression t = e[i];
      s_expression_ref src, dest;
      if (s_pattern("edge", src, dest) ^ t) {
        AR_Node_Ref< AR_Basic_Block > src_bblock =
            name_to_bblock[((static_cast< string_atom& >(**src)).data())];
        AR_Node_Ref< AR_Basic_Block > dest_bblock =
            name_to_bblock[((static_cast< string_atom& >(**dest)).data())];
        assert(src_bblock != AR_Null_Ref && dest_bblock != AR_Null_Ref);
        src_bblock->addNextBasicBlock(dest_bblock);
      } else {
        throw parse_error("AR_Code::connect_basic_blocks() parsing edge", e);
      }
    }
  } else {
    throw parse_error("AR_Code::connect_basic_blocks() parsing trans", e);
  }
}

AR_Code::~AR_Code() {}

void AR_Code::print(std::ostream& out) {
  AR_Node_Ref< AR_Basic_Block > entry(_entry_block);
  AR_Node_Ref< AR_Basic_Block > exit(_exit_block);
  AR_Node_Ref< AR_Basic_Block > unreachable(_unreachable_block);
  AR_Node_Ref< AR_Basic_Block > ehresume(_ehresume_block);

  out << "entry: " << (*entry).getNameId() << "!" << entry.getUID()
      << std::endl;

  if (_exit_block != 0) {
    out << "exit: " << (*exit).getNameId() << "!" << exit.getUID() << std::endl;
  } else {
    out << "exit: none" << std::endl;
  }

  if (_unreachable_block != 0) {
    out << "unreachable: " << (*unreachable).getNameId() << "!"
        << unreachable.getUID() << std::endl;
  } else {
    out << "unreachable: none" << std::endl;
  }

  if (_ehresume_block != 0) {
    out << "ehresume: " << (*ehresume).getNameId() << "!" << ehresume.getUID()
        << std::endl;
  } else {
    out << "ehresume: none" << std::endl;
  }

  out << std::endl;
  typedef std::vector< AR_Node_Ref< AR_Basic_Block > >::iterator BBIt;
  for (BBIt I = _blocks.begin(), E = _blocks.end(); I != E; ++I) {
    (**I).print(out);
    out << std::endl;
  }
}

////
// AR_Function
////

AR_Function::AR_Function(
    const std::string& name_id,
    AR_Node_Ref< AR_Function_Type > function_type,
    const std::vector< AR_Node_Ref< AR_Internal_Variable > >& parameters)
    : _name_id(name_id),
      _function_body(AR_Null_Ref),
      _function_type(function_type),
      _parameters(parameters) {
  ARModel::Instance()->setCurrentBuildingFunction(
      AR_Node_Ref< AR_Function >(getUID()));
  _function_body = AR_Code::create(AR_Node_Ref< AR_Function >(getUID()));
}

AR_Function::AR_Function(s_expression e)
    : _function_body(AR_Null_Ref), _function_type(AR_Null_Ref) {
  ARModel::Instance()->setCurrentBuildingFunction(
      AR_Node_Ref< AR_Function >(getUID()));
  /**
   * s-expr: ($function ($name ()) ($ty ()) ($params () ...) ($local_vars ()
   * ...) ($code ($basicblock) ...))
   */
  s_expression_ref n, ty, params, lv, code;
  if (s_pattern("function",
                s_pattern("name", n),
                s_pattern("ty", ty),
                params,
                lv,
                code) ^
      e) {
    // Get function name
    _name_id = (static_cast< string_atom& >(**n)).data();

    // Get function type
    index64_t ty_id = (static_cast< index64_atom& >(**ty)).data();
    _function_type =
        node_cast< AR_Function_Type >(ARFactory::createType(ty_id));

    // Collect formal parameters; formal parameters are represented as
    // AR_Local_Variable
    for (size_t i = 1; i <= (*params).n_args(); i++) {
      // Example param s-expr: ($p ($name ($argc)) ($ty (!11)))
      s_expression p = (*params)[i];
      s_expression_ref n, ty;
      if (s_pattern("p", s_pattern("name", n), s_pattern("ty", ty)) ^ p) {
        std::string name = (static_cast< string_atom& >(**n)).data();
        index64_t ty_id = (static_cast< index64_atom& >(**ty)).data();
        AR_Node_Ref< AR_Type > type = ARFactory::createType(ty_id);
        AR_Node_Ref< AR_Internal_Variable > param =
            AR_Internal_Variable::create(type, name);
        _parameters.push_back(param);
      } else {
        throw parse_error("AR_Local_Variable", p);
      }
    }

    // Collect local variables
    for (size_t i = 1; i <= (*lv).n_args(); i++) {
      AR_Node_Ref< AR_Local_Variable > local_var =
          AR_Local_Variable::create((*lv)[i]);
      _local_variables.push_back(local_var);
    }

    _function_body = ARFactory::createCode(getUID(), *code);
  } else {
    throw parse_error("AR_Function", e);
  }
}

const std::string AR_Function::BASICBLOCK_TAG = "($basicblock";

AR_Function::~AR_Function() {}

void AR_Function::print(std::ostream& out) {
  out << "def func!" << getUID() << " ";
  _function_type->getReturnType()->print(out);
  out << " " << _name_id << "(";
  std::vector< AR_Node_Ref< AR_Internal_Variable > >::iterator it =
      _parameters.begin();
  while (it != _parameters.end()) {
    (**it).print(out);
    it++;
    out << ((it == _parameters.end()) ? ")" : ", ");
  }

  std::cout << std::endl << std::endl;

  std::vector< AR_Node_Ref< AR_Local_Variable > >::iterator j =
      _local_variables.begin();
  for (; j != _local_variables.end(); j++) {
    (**j).print(out);
    out << std::endl;
  }

  out << std::endl;

  (*_function_body).print(out);
  out << std::endl;
}

AR_Node_Ref< AR_Internal_Variable > AR_Function::getFormalParameter(
    const std::string& name) const {
  for (auto it = _parameters.begin(); it != _parameters.end(); it++) {
    if ((*it)->getName() == name) {
      return *it;
    }
  }
  return AR_Null_Ref;
}

AR_Node_Ref< AR_Local_Variable > AR_Function::getLocalVariable(
    const std::string& name) const {
  for (auto it = _local_variables.begin(); it != _local_variables.end(); it++) {
    if ((*it)->getNameId() == name) {
      return *it;
    }
  }
  return AR_Null_Ref;
}

void AR_Function::addLocalVariable(AR_Node_Ref< AR_Local_Variable > local_var) {
  _local_variables.push_back(local_var);
}

namespace {

class CallStatementVisitor : public Visitor {
private:
  std::vector< index64_t >& _callees;
  const FunPointersInfo& _fun_ptr_info;

public:
  CallStatementVisitor(std::vector< index64_t >& callees,
                       const FunPointersInfo& fun_ptr_info)
      : _callees(callees), _fun_ptr_info(fun_ptr_info) {}

  virtual void nodeStart(AR_Call& call) {
    // ignore external calls
    AR_Node_Ref< AR_Operand > operand = call.getFunction();
    if (operand->getClassType() == AR_CST_OPERAND_CLASS_TYPE) {
      AR_Node_Ref< AR_Cst_Operand > cst_operand =
          node_cast< AR_Cst_Operand >(operand);
      AR_Node_Ref< AR_Function_Addr_Constant > function_addr =
          node_cast< AR_Function_Addr_Constant >(cst_operand->getConstant());
      AR_Node_Ref< AR_Function > function = function_addr->getFunction();
      if (function.getUID() != 0 && function->getFunctionBody().getUID() != 0) {
        _callees.push_back(function.getUID());
      }
    } else if (operand->getClassType() == AR_VAR_OPERAND_CLASS_TYPE) {
      AR_Node_Ref< AR_Var_Operand > var_operand =
          node_cast< AR_Var_Operand >(operand);
      std::vector< AR_Node_Ref< AR_Function > > pointsto =
          _fun_ptr_info.pointedBy(var_operand->getInternalVariable());
      for (std::vector< AR_Node_Ref< AR_Function > >::iterator it =
               pointsto.begin();
           it != pointsto.end();
           ++it) {
        AR_Node_Ref< AR_Function > function(*it);
        if (function.getUID() != 0 &&
            function->getFunctionBody().getUID() != 0) {
          _callees.push_back(function.getUID());
        }
      }
    }
  }

  virtual void nodeStart(AR_Invoke& invoke) {
    AR_Node_Ref< AR_Call > call = invoke.getFunctionCall();
    nodeStart(*call);
  }
};

} // end anonymous namespace

std::vector< index64_t > AR_Function::buildCallees(
    const FunPointersInfo& fun_ptr_info) const {
  std::vector< index64_t > callees;
  std::shared_ptr< CallStatementVisitor > visitor(
      new CallStatementVisitor(callees, fun_ptr_info));
  _function_body->accept(visitor);
  return callees;
}

////
// AR_Bundle
////

AR_Bundle::~AR_Bundle() {}

boost::optional< AR_Node_Ref< AR_Function > > AR_Bundle::getFunctionByNameId(
    const std::string& name) const {
  index64_t uid = ARModel::Instance()->getFunctionUIDByNameId(name);
  if (uid == 0) {
    return boost::optional< AR_Node_Ref< AR_Function > >();
  } else {
    return boost::optional< AR_Node_Ref< AR_Function > >(
        AR_Node_Ref< AR_Function >(uid));
  }
}

void AR_Bundle::addGlobalVariable(AR_Node_Ref< AR_Global_Variable > gv) {
  _globals.push_back(gv);
}

void AR_Bundle::addFunction(AR_Node_Ref< AR_Function > f) {
  _functions.push_back(f);
  ARModel::Instance()->registerFunction(f);
}

void AR_Bundle::buildCallGraph(const FunPointersInfo& fun_ptr_info) {
  typedef std::vector< AR_Node_Ref< AR_Function > >::iterator FuncIt;
  for (FuncIt it = _functions.begin(); it != _functions.end(); ++it) {
    AR_Node_Ref< AR_Function > caller = *it;
    std::vector< index64_t > callees = caller->buildCallees(fun_ptr_info);
    for (std::vector< index64_t >::iterator it2 = callees.begin();
         it2 != callees.end();
         ++it2) {
      AR_Node_Ref< AR_Function > callee(*it2);
      caller->_callees.insert(callee.getUID());
      callee->_callers.insert(caller.getUID());
    }
  }
}

void AR_Bundle::cleanCallGraph() {
  typedef std::vector< AR_Node_Ref< AR_Function > >::iterator FuncIt;
  for (FuncIt it = _functions.begin(), et = _functions.end(); it != et; ++it) {
    AR_Node_Ref< AR_Function > fun = *it;
    fun->_callers.clear();
    fun->_callees.clear();
  }
}

void AR_Bundle::print(std::ostream& out) {
  // Print metainfo
  out << std::endl << "def bundle!" << getUID() << std::endl;

  out << std::endl;
  out << "# TargetTriple = " << _target_triple << std::endl;
  out << "# isLittleEndian = " << (_is_little_endian ? "True" : "False")
      << std::endl;
  out << "# sizeofptr = " << _sizeofptr << std::endl;

  out << std::endl;

  // Print file paths
  std::unordered_map< index64_t, std::string > files =
      ARModel::Instance()->getFiles();
  std::unordered_map< index64_t, std::string >::iterator f = files.begin();
  for (; f != files.end(); f++) {
    out << "def file!" << f->first << " " << f->second << std::endl;
  }

  out << std::endl;

  // Print global variables
  typedef std::vector< AR_Node_Ref< AR_Global_Variable > >::iterator GVIt;
  for (GVIt I = _globals.begin(), E = _globals.end(); I != E; ++I) {
    (**I).print(out);
    out << std::endl;
  }

  out << std::endl;

  // Print struct types
  std::vector< index64_t > visited;
  std::list< AR_Node_Ref< AR_Node > > flyweights =
      ARModel::Instance()->getARFlyweights();
  std::list< AR_Node_Ref< AR_Node > >::iterator i = flyweights.begin();
  for (; i != flyweights.end(); i++) {
    if ((**i).getClassType() == AR_STRUCT_TYPE_CLASS_TYPE) {
      AR_Node_Ref< AR_Structure_Type > structty =
          node_cast< AR_Structure_Type >(*i);
      (*structty).print_layout(visited, out);
      out << std::endl;
    } else if ((**i).getClassType() == AR_FUNCTION_TYPE_CLASS_TYPE) {
      AR_Node_Ref< AR_Function_Type > functy =
          node_cast< AR_Function_Type >(*i);
      (*functy).print(out);
    }
  }

  out << std::endl << std::endl;

  // Print functions
  typedef std::vector< AR_Node_Ref< AR_Function > >::iterator FuncIt;
  for (FuncIt I = _functions.begin(), E = _functions.end(); I != E; ++I) {
    (**I).print(out);
    out << std::endl;
  }

  out << std::endl;
}

////
// AR_Source_Location
////

AR_Source_Location::AR_Source_Location(index64_t fid,
                                       const std::string& filename,
                                       z_number line,
                                       z_number column)
    : _fid(fid), _filename(filename), _line(line), _column(column) {}

AR_Source_Location::AR_Source_Location(s_expression e)
    : _fid(-1), _filename("?"), _line(-1), _column(-1) {
  /**
   * Example s-expr: ($srcloc ($line (#4)) ($col (#3)) ($file (!9)))
   */
  s_expression_ref l, c, f;
  if ((s_pattern("srcloc",
                 s_pattern("line", l),
                 s_pattern("col", c),
                 s_pattern("file", f))) ^
      e) {
    _line = (static_cast< z_number_atom& >(**l)).data();
    _column = (static_cast< z_number_atom& >(**c)).data();
    _fid = (static_cast< index64_atom& >(**f)).data();
    _filename = ARModel::Instance()->get_filepath(_fid);
  } else if (e.n_args() == 0) {
    // This is a valid case.
  } else {
    throw parse_error("AR_Source_Location", e);
  }
}

AR_Source_Location::~AR_Source_Location() {}

void AR_Source_Location::print(std::ostream& out) {
  out << "@file!" << _fid << ":" << _line << ":" << _column;
}

////
// ARModel
////

void ARModel::generate(std::istream& is) {
  if (is.peek() == std::ifstream::traits_type::eof()) {
    throw error("Input file is empty.");
  }

  setBundle(AR_Bundle::create());

  s_expression_istream sis(is);
  for (;;) {
    s_expression_ref e;
    sis >> e;
    if (e) {
      s_expression expr = *e;
      s_expression_ref a;
      if ((s_pattern("metainfo", a) ^ expr)) {
        process_meta_info(*a);
      } else if ((s_pattern("def", a) ^ expr)) {
        index64_t ref = (static_cast< index64_atom& >(**a)).data();
        _ordered_refs.push_back(ref);
        _input_ref_to_sexpr[ref] = (*a)[1];
      } else if ((*expr).atom_type() == STRING_ATOM) {
        string_atom& str_atom = static_cast< string_atom& >(*expr);
        std::string kind = str_atom.data();
        if (kind == FUNCTION) {
          process_global_defs();
          (*_bundle).addFunction(ARFactory::createFunction(expr));
        }
      }
    } else {
      break;
    }
  }

  std::vector< index64_t >::iterator i =
      _listeners[ARModelEventListener::NODESCREATED].begin();
  for (; i != _listeners[ARModelEventListener::NODESCREATED].end(); i++) {
    ARModelEventListener* cb_ptr = dynamic_cast< ARModelEventListener* >(
        ARModel::Instance()->getARNode(*i));
    cb_ptr->nodesCreated();
  }
  _listeners.clear();

  _ordered_refs.clear();
  _input_ref_to_sexpr.clear();
  _input_ref_to_ar_map.clear();
  _current_building_scope = AR_Node_Ref< AR_Code >(0);
  _complete = true;
}

void ARModel::process_meta_info(s_expression e) {
  // Example input: ($endianess ($LittleEndian))
  s_expression_ref p;
  if (s_pattern("endianess", p) ^ e) {
    std::string endianess = (static_cast< string_atom& >(**p)).data();
    (*_bundle).setEndianess(endianess == LITTLEENDIAN);
  } else if (s_pattern("sizeofptr", p) ^ e) {
    (*_bundle).set_sizeofptr((static_cast< z_number_atom& >(**p)).data());
  } else if (s_pattern("targettriple", p) ^ e) {
    std::string target_triple = (static_cast< string_atom& >(**p)).data();
    (*_bundle).setTargetTriple(target_triple);
  }
}

void ARModel::process_global_defs() {
  // Check if all global defs have been processed
  if (_input_ref_to_ar_map.size() > 0) {
    return;
  }

  std::vector< index64_t >::iterator i = _ordered_refs.begin();
  for (; i != _ordered_refs.end(); i++) {
    index64_t id = *i;
    if (getARfromInputRef(id) == 0) {
      s_expression e = *_input_ref_to_sexpr[id];
      s_expression_ref a;
      if ((s_pattern(TYPE, a) ^ e)) {
        ARFactory::createType(id);
      } else if ((s_pattern("file", a) ^ e)) {
        _file_map[id] = (static_cast< string_atom& >(**a)).data();
      } else if ((s_pattern("gv") ^ e)) {
        ARFactory::createGlobalVariable(id);
      } else {
        std::cerr << e << " not processed" << std::endl;
      }
    }
  }
}

void ARModel::print(std::ostream& out) {
  (*_bundle).print(out);
}

std::string ReferenceCounter::str(AR_CLASS_TYPE_CODE class_type) {
  switch (class_type) {
    case AR_BUNDLE_CLASS_TYPE:
      return "AR_Bundle";
    case AR_FUNCTION_CLASS_TYPE:
      return "AR_Function";
    case AR_CODE_CLASS_TYPE:
      return "AR_Code";
    case AR_BASIC_BLOCK_CLASS_TYPE:
      return "AR_Basic_Block";
    case AR_CONV_OP_STATEMENT_CLASS_TYPE:
      return "AR_Conv_Op";
    case AR_BITWISE_OP_STATEMENT_CLASS_TYPE:
      return "AR_Bitwise_Op";
    case AR_INTEGER_COMPARISON_STATEMENT_CLASS_TYPE:
      return "AR_Integer_Comparison";
    case AR_FP_COMPARISON_STATEMENT_CLASS_TYPE:
      return "AR_FP_Comparison";
    case AR_ASSIGNMENT_STATEMENT_CLASS_TYPE:
      return "AR_Assignment";
    case AR_CALL_STATEMENT_CLASS_TYPE:
      return "AR_Call";
    case AR_INVOKE_STATEMENT_CLASS_TYPE:
      return "AR_Invoke";
    case AR_ALLOCATE_STATEMENT_CLASS_TYPE:
      return "AR_Allocate";
    case AR_STORE_STATEMENT_CLASS_TYPE:
      return "AR_Store";
    case AR_LOAD_STATEMENT_CLASS_TYPE:
      return "AR_Load";
    case AR_INSERT_ELEMENT_STATEMENT_CLASS_TYPE:
      return "AR_Insert_Element";
    case AR_EXTRACT_ELEMENT_STATEMENT_CLASS_TYPE:
      return "AR_Extract_Element";
    case AR_POINTER_SHIFT_STATEMENT_CLASS_TYPE:
      return "AR_Pointer_Shift";
    case AR_RETURN_VALUE_STATEMENT_CLASS_TYPE:
      return "AR_Return_Value";
    case AR_INTEGER_ARITH_OP_STATEMENT_CLASS_TYPE:
      return "AR_Arith_Op";
    case AR_FP_ARITH_OP_STATEMENT_CLASS_TYPE:
      return "AR_FP_Op";
    case AR_ABSTRACT_VAR_STATEMENT_CLASS_TYPE:
      return "AR_Abstract_Variable";
    case AR_ABSTRACT_MEM_STATEMENT_CLASS_TYPE:
      return "AR_Abstract_Memory";
    case AR_LANDING_PAD_STATEMENT_CLASS_TYPE:
      return "AR_Landing_Pad";
    case AR_RESUME_STATEMENT_CLASS_TYPE:
      return "AR_Resume";
    case AR_VA_START_STATEMENT_CLASS_TYPE:
      return "AR_VA_Start";
    case AR_VA_END_STATEMENT_CLASS_TYPE:
      return "AR_VA_End";
    case AR_VA_COPY_STATEMENT_CLASS_TYPE:
      return "AR_VA_Copy";
    case AR_VA_ARG_STATEMENT_CLASS_TYPE:
      return "AR_VA_Arg";
    case AR_UNREACHABLE_STATEMENT_CLASS_TYPE:
      return "AR_Unreachable";
    case AR_NOP_STATEMENT_CLASS_TYPE:
      return "AR_NOP";
    case AR_MEMCPY_STATEMENT_CLASS_TYPE:
      return "AR_MemCpy";
    case AR_MEMMOVE_STATEMENT_CLASS_TYPE:
      return "AR_MemMove";
    case AR_MEMSET_STATEMENT_CLASS_TYPE:
      return "AR_MemSet";
    case AR_INTEGER_TYPE_CLASS_TYPE:
      return "AR_Integer_Type";
    case AR_INTEGER_CONSTANT_CLASS_TYPE:
      return "AR_Integer_Constant";
    case AR_FLOAT_TYPE_CLASS_TYPE:
      return "AR_FP_Type";
    case AR_FLOAT_CONSTANT_CLASS_TYPE:
      return "AR_FP_Constant";
    case AR_FUNCTION_TYPE_CLASS_TYPE:
      return "AR_Function_Type";
    case AR_POINTER_TYPE_CLASS_TYPE:
      return "AR_Pointer_Type";
    case AR_ARRAY_TYPE_CLASS_TYPE:
      return "AR_Array_Type";
    case AR_STRUCT_TYPE_CLASS_TYPE:
      return "AR_Structure_Type";
    case AR_RANGE_CONSTANT_CLASS_TYPE:
      return "AR_Range_Constant";
    case AR_UNDEFINED_CONSTANT_CLASS_TYPE:
      return "AR_Undefined_Constant";
    case AR_CST_OPERAND_CLASS_TYPE:
      return "AR_Cst_Operand";
    case AR_VAR_OPERAND_CLASS_TYPE:
      return "AR_Var_Operand";
    case AR_FUNCTION_ADDR_CONSTANT_CLASS_TYPE:
      return "AR_Function_Addr_Constant";
    case AR_VAR_ADDR_CONSTANT_CLASS_TYPE:
      return "AR_Var_Addr_Constant";
    case AR_NULL_CONSTANT_CLASS_TYPE:
      return "AR_Null_Constant";
    case AR_INTERNAL_VAR_CLASS_TYPE:
      return "AR_Internal_Variable";
    case AR_VOID_TYPE_CLASS_TYPE:
      return "AR_Void_Type";
    case AR_OPAQUE_TYPE_CLASS_TYPE:
      return "AR_Opaque_Type";
    case AR_LOCAL_VAR_CLASS_TYPE:
      return "AR_Local_Variable";
    case AR_GLOBAL_VAR_CLASS_TYPE:
      return "AR_Global_Variable";
    case AR_SOURCE_LOCATION_CLASS_TYPE:
      return "AR_Source_Location";
    default:
      return EM_STR;
  }
}

std::string ARModel::METAINFO = "metainfo";
std::string ARModel::FUNCTION = "function";
std::string ARModel::ENDIANESS = "endianess";
std::string ARModel::LITTLEENDIAN = "LittleEndian";
std::string ARModel::SIZEOFPTR = "sizeofptr";
std::string ARModel::TYPE = "type";

/**
 * Options
 */

// OptionRegistry
OptionRegistry* OptionRegistry::_instance = nullptr;

// OptionBase
void OptionBase::registerOption() {
  OptionRegistry::Get()->add(*this);
}

void OptionBase::unregisterOption() {
  OptionRegistry::Get()->remove(*this);
}
