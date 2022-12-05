/*******************************************************************************
 *
 * \file
 * \brief OperandsTable implementation
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2018-2019 United States Government as represented by the
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

#include <regex>
#include <vector>

#include <boost/container/flat_set.hpp>

#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/Transforms/Utils/Local.h>

#include <ikos/ar/semantic/function.hpp>
#include <ikos/ar/semantic/value.hpp>
#include <ikos/ar/semantic/value_visitor.hpp>

#include <ikos/analyzer/database/table/functions.hpp>
#include <ikos/analyzer/database/table/operands.hpp>
#include <ikos/analyzer/exception.hpp>
#include <ikos/analyzer/util/demangle.hpp>

namespace ikos {
namespace analyzer {

OperandsTable::OperandsTable(sqlite::DbConnection& db)
    : DatabaseTable(db,
                    "operands",
                    {{"id", sqlite::DbColumnType::Integer},
                     {"kind", sqlite::DbColumnType::Integer},
                     {"repr", sqlite::DbColumnType::Text}},
                    {}),
      _row(db, "operands", 3) {}

sqlite::DbInt64 OperandsTable::insert(ar::Value* value) {
  ikos_assert(value != nullptr);

  auto it = this->_map.find(value);
  if (it != this->_map.end()) {
    return it->second;
  }

  sqlite::DbInt64 id = this->_last_insert_id++;
  this->_row << id;
  this->_row << static_cast< sqlite::DbInt64 >(value->kind());
  this->_row << repr(value);
  this->_row << sqlite::end_row;

  this->_map.try_emplace(value, id);
  return id;
}

namespace detail {
namespace {

/// \brief Return true if the given expression has parentheses
bool has_parentheses(StringRef s) {
  return s.size() > 2 && *s.begin() == '(' && *s.rbegin() == ')';
}

/// \brief Add parentheses around the given expression
std::string add_parentheses(StringRef s) {
  ikos_assert(!s.empty());

  static std::regex ArrayPattern(
      "&?[a-zA-Z0-9_]+("
      "(\\[.*\\])"
      "|(\\->[a-zA-Z0-9_]+)"
      "|(\\.[a-zA-Z0-9_]+)"
      ")*");

  static std::regex FunctionPattern(
      "&?[a-zA-Z0-9_]+\\(([a-zA-Z0-9_]+(, )?)*\\)");

  static std::regex DerefPattern("\\*[a-zA-Z0-9_]+");

  if (std::regex_match(s.begin(), s.end(), ArrayPattern) ||
      std::regex_match(s.begin(), s.end(), FunctionPattern) ||
      std::regex_match(s.begin(), s.end(), DerefPattern)) {
    // no need for parentheses
    return s.to_string();
  }

  // else
  return "(" + s.to_string() + ")";
}

/// \brief Remove parentheses around the given expression
StringRef remove_parentheses(StringRef s) {
  ikos_assert(!s.empty());

  if (has_parentheses(s)) {
    s = s.substr(1, s.size() - 2);
  }

  return s;
}

/// \brief Return true if the given expression is an address
bool is_address(StringRef s) {
  return !s.empty() && s[0] == '&';
}

/// \brief Add a dereference
std::string add_deref(StringRef s) {
  ikos_assert(!s.empty());

  if (is_address(s)) {
    // s = `&x`
    return remove_parentheses(s.substr(1)).to_string();
  } else {
    return "*" + add_parentheses(s);
  }
}

/// \brief Convert a llvm::APInt to a string
std::string to_string(const llvm::APInt& n) {
  llvm::SmallString< 16 > str;
  n.toString(str, /*radix=*/10, /*signed=*/true);
  return str.str().str();
}

/// \brief Convert a llvm::APFloat to a string
std::string to_string(const llvm::APFloat& f) {
  llvm::SmallString< 16 > str;
  f.toString(str, /*FormatPrecision=*/0, /*FormatMaxPadding=*/0);
  return str.str().str();
}

/// \brief Return the hexadecimal character for the given number
char hexdigit(unsigned n, bool lower_case = false) {
  if (n < 10U) {
    return static_cast< char >('0' + n);
  } else if (n < 16U) {
    return static_cast< char >((lower_case ? 'a' : 'A') + (n - 10));
  } else {
    ikos_unreachable("invalid argument");
  }
}

/// \brief Return the escaped string
std::string escape_string(llvm::StringRef s) {
  std::string r;
  r.reserve(s.size() + 2);
  r.push_back('"');
  for (auto it = s.begin(), et = s.end(); it != et; ++it) {
    const char c = *it;
    if (c == '"' || c == '\\') {
      r.push_back('\\');
      r.push_back(c);
    } else if (std::isprint(c) != 0) {
      r.push_back(c);
    } else if (c == '\b') {
      r.append("\\b");
    } else if (c == '\t') {
      r.append("\\t");
    } else if (c == '\n') {
      r.append("\\n");
    } else if (c == '\f') {
      r.append("\\f");
    } else if (c == '\r') {
      r.append("\\r");
    } else if (c == '\0' && std::next(it) == et) {
      continue; // Skip ending null-byte
    } else {
      r.push_back('\\');
      r.push_back(hexdigit(static_cast< unsigned char >(c) >> 4U));
      r.push_back(hexdigit(static_cast< unsigned char >(c) & 0x0FU));
    }
  }
  r.push_back('"');
  return r;
}

/// \brief Return the textual representation of a binary operator
StringRef repr(llvm::Instruction::BinaryOps op) {
  switch (op) {
    case llvm::Instruction::Add:
      return "+";
    case llvm::Instruction::FAdd:
      return "+";
    case llvm::Instruction::Sub:
      return "-";
    case llvm::Instruction::FSub:
      return "-";
    case llvm::Instruction::Mul:
      return "*";
    case llvm::Instruction::FMul:
      return "*";
    case llvm::Instruction::UDiv:
      return "/";
    case llvm::Instruction::SDiv:
      return "/";
    case llvm::Instruction::FDiv:
      return "/";
    case llvm::Instruction::URem:
      return "%";
    case llvm::Instruction::SRem:
      return "%";
    case llvm::Instruction::FRem:
      return "%";
    case llvm::Instruction::Shl:
      return "<<";
    case llvm::Instruction::LShr:
      return ">>";
    case llvm::Instruction::AShr:
      return ">>";
    case llvm::Instruction::And:
      return "&";
    case llvm::Instruction::Or:
      return "|";
    case llvm::Instruction::Xor:
      return "^";
    default:
      throw FrontendError("unsupported llvm binary operator");
  }
}

/// \brief Return the textual representation of a CmpInst predicate
StringRef repr(llvm::CmpInst::Predicate pred) {
  switch (pred) {
    case llvm::CmpInst::FCMP_FALSE:
      return "false";
    case llvm::CmpInst::FCMP_OEQ:
      return "==";
    case llvm::CmpInst::FCMP_OGT:
      return ">";
    case llvm::CmpInst::FCMP_OGE:
      return ">=";
    case llvm::CmpInst::FCMP_OLT:
      return "<";
    case llvm::CmpInst::FCMP_OLE:
      return "<=";
    case llvm::CmpInst::FCMP_ONE:
      return "!=";
    case llvm::CmpInst::FCMP_ORD:
      return "not_nan";
    case llvm::CmpInst::FCMP_UNO:
      return "is_nan";
    case llvm::CmpInst::FCMP_UEQ:
      return "==";
    case llvm::CmpInst::FCMP_UGT:
      return ">";
    case llvm::CmpInst::FCMP_UGE:
      return ">=";
    case llvm::CmpInst::FCMP_ULT:
      return "<";
    case llvm::CmpInst::FCMP_ULE:
      return "<=";
    case llvm::CmpInst::FCMP_UNE:
      return "!=";
    case llvm::CmpInst::FCMP_TRUE:
      return "true";
    case llvm::CmpInst::ICMP_EQ:
      return "==";
    case llvm::CmpInst::ICMP_NE:
      return "!=";
    case llvm::CmpInst::ICMP_UGT:
      return ">";
    case llvm::CmpInst::ICMP_UGE:
      return ">=";
    case llvm::CmpInst::ICMP_ULT:
      return "<";
    case llvm::CmpInst::ICMP_ULE:
      return "<=";
    case llvm::CmpInst::ICMP_SGT:
      return ">";
    case llvm::CmpInst::ICMP_SGE:
      return ">=";
    case llvm::CmpInst::ICMP_SLT:
      return "<";
    case llvm::CmpInst::ICMP_SLE:
      return "<=";
    default:
      throw FrontendError("unsupported llvm cmp predicate");
  }
}

/// \brief Set of types
using TypeSet = boost::container::flat_set< llvm::Type* >;

/// \brief Set of values
using ValueSet = boost::container::flat_set< llvm::Value* >;

/// \brief Return the textual representation of a llvm::Type*
std::string repr(llvm::Type* type, TypeSet seen) {
  ikos_assert(type != nullptr);

  if (type->isVoidTy()) {
    return "void";
  } else if (auto int_type = llvm::dyn_cast< llvm::IntegerType >(type)) {
    if (int_type->getBitWidth() == 1) {
      return "bool";
    } else {
      return "int" + std::to_string(int_type->getBitWidth()) + "_t";
    }
  } else if (type->isFloatingPointTy()) {
    if (type->isHalfTy()) {
      return "half_t";
    } else if (type->isFloatTy()) {
      return "float";
    } else if (type->isDoubleTy()) {
      return "double";
    } else if (type->isX86_FP80Ty()) {
      return "fp80_t";
    } else if (type->isFP128Ty()) {
      return "fp128_t";
    } else if (type->isPPC_FP128Ty()) {
      return "ppc_fp128_t";
    } else {
      throw FrontendError("unsupported llvm floating point type");
    }
  } else if (auto ptr_type = llvm::dyn_cast< llvm::PointerType >(type)) {
    return repr(ptr_type->getPointerElementType(), seen) + "*";
  } else if (auto array_type = llvm::dyn_cast< llvm::ArrayType >(type)) {
    return repr(array_type->getElementType(), seen) + "[" +
           std::to_string(array_type->getNumElements()) + "]";
  } else if (auto vector_type = llvm::dyn_cast< llvm::VectorType >(type)) {
    return repr(vector_type->getElementType(), seen) + "[" +
           std::to_string(vector_type->getElementCount().getFixedValue()) + "]";
  } else if (auto struct_type = llvm::dyn_cast< llvm::StructType >(type)) {
    if (struct_type->hasName()) {
      llvm::StringRef name = struct_type->getName();
      name.consume_front("struct.");
      name.consume_front("class.");
      return name.str();
    }

    if (struct_type->isOpaque()) {
      return "{...}";
    }

    // Avoid infinite recursion
    auto p = seen.insert(type);
    if (!p.second) {
      return "{...}"; // already processing
    }

    std::string r = "{";
    for (auto it = struct_type->element_begin(),
              et = struct_type->element_end();
         it != et;) {
      r += repr(*it, seen);
      ++it;
      if (it != et) {
        r += ", ";
      }
    }
    r += "}";
    return r;
  } else if (auto fun_type = llvm::dyn_cast< llvm::FunctionType >(type)) {
    std::string r = repr(fun_type->getReturnType(), seen);
    r += "(";
    for (auto it = fun_type->param_begin(), et = fun_type->param_end();
         it != et;) {
      r += repr(*it, seen);
      ++it;
      if (it != et) {
        r += ", ";
      }
    }
    r += ")";
    return r;
  } else {
    throw FrontendError("unsupported llvm type");
  }
}

/// \brief Remove any typedef/const/volatile/etc. qualifier
llvm::DIType* remove_qualifiers(llvm::DIType* type) {
  while (type != nullptr && !type->isForwardDecl() &&
         llvm::isa< llvm::DIDerivedType >(type)) {
    auto derived_type = llvm::cast< llvm::DIDerivedType >(type);
    if (derived_type->getTag() == llvm::dwarf::DW_TAG_typedef ||
        derived_type->getTag() == llvm::dwarf::DW_TAG_const_type ||
        derived_type->getTag() == llvm::dwarf::DW_TAG_volatile_type ||
        derived_type->getTag() == llvm::dwarf::DW_TAG_restrict_type ||
        derived_type->getTag() == llvm::dwarf::DW_TAG_atomic_type) {
      type = llvm::cast_or_null< llvm::DIType >(derived_type->getRawBaseType());
    } else {
      break;
    }
  }
  return type;
}

/// \brief Return the pointee type of a pointer type, or nullptr
llvm::DIType* pointee_type(llvm::DIType* type) {
  type = remove_qualifiers(type);

  if (type == nullptr) {
    return nullptr;
  }

  if (auto derived_type = llvm::dyn_cast< llvm::DIDerivedType >(type)) {
    if (derived_type->getTag() == llvm::dwarf::DW_TAG_pointer_type) {
      return llvm::cast_or_null< llvm::DIType >(derived_type->getRawBaseType());
    }
  }

  return nullptr;
}

/// \brief Result of a repr() function
struct ReprResult {
public:
  /// \brief Textual representation
  std::string str;

  /// \brief Debug info pointee type (or nullptr)
  llvm::DIType* pointee_type;

public:
  /// \brief Constructor
  explicit ReprResult(std::string str_, llvm::DIType* pointee_type_ = nullptr)
      : str(std::move(str_)), pointee_type(pointee_type_) {
    ikos_assert(!this->str.empty());
  }

}; // end struct ReprResult

// Forward declaration
ReprResult repr(llvm::Constant*, ValueSet seen);
ReprResult repr(llvm::Value*, ValueSet seen);

/// \brief Return the textual representation of a llvm::Constant*
ReprResult repr(llvm::Constant* cst, ValueSet seen) {
  if (auto gv_alias = llvm::dyn_cast< llvm::GlobalAlias >(cst)) {
    return repr(gv_alias->getAliasee(), seen);
  } else if (auto gv = llvm::dyn_cast< llvm::GlobalVariable >(cst)) {
    // Check for debug info
    llvm::SmallVector< llvm::DIGlobalVariableExpression*, 1 > dbgs;
    gv->getDebugInfo(dbgs);

    if (!dbgs.empty()) {
      llvm::DIGlobalVariable* di_gv = dbgs[0]->getVariable();
      auto di_type = llvm::cast_or_null< llvm::DIType >(di_gv->getRawType());
      llvm::StringRef name = di_gv->getName();

      if (!name.empty()) {
        return ReprResult{"&" + demangle(name), di_type};
      }
    }

    // If it's a constant (e.g, a string)
    if (gv->isConstant() && gv->hasInitializer()) {
      return ReprResult{"&" + repr(gv->getInitializer(), seen).str};
    }

    // Last chance, use llvm variable name
    if (gv->hasName()) {
      return ReprResult{"&" + demangle(gv->getName())};
    }

    return ReprResult{"&__unnamed_global_var"};
  } else if (auto fun = llvm::dyn_cast< llvm::Function >(cst)) {
    return ReprResult{"&" + demangle(FunctionsTable::name(fun))};
  } else if (auto cst_int = llvm::dyn_cast< llvm::ConstantInt >(cst)) {
    return ReprResult{detail::to_string(cst_int->getValue())};
  } else if (auto cst_fp = llvm::dyn_cast< llvm::ConstantFP >(cst)) {
    return ReprResult{detail::to_string(cst_fp->getValueAPF())};
  } else if (llvm::isa< llvm::ConstantPointerNull >(cst)) {
    return ReprResult{"NULL"};
  } else if (llvm::isa< llvm::UndefValue >(cst)) {
    return ReprResult{"undefined"};
  } else if (llvm::isa< llvm::ConstantAggregateZero >(cst)) {
    return ReprResult{"{0}"};
  } else if (auto cst_array = llvm::dyn_cast< llvm::ConstantArray >(cst)) {
    std::string r = "[";
    for (auto it = cst_array->op_begin(), et = cst_array->op_end(); it != et;) {
      r += repr(*it, seen).str;
      ++it;
      if (it != et) {
        r += ", ";
      }
    }
    r += "]";
    return ReprResult{r};
  } else if (auto cst_struct = llvm::dyn_cast< llvm::ConstantStruct >(cst)) {
    std::string r = "{";
    for (auto it = cst_struct->op_begin(), et = cst_struct->op_end();
         it != et;) {
      r += repr(*it, seen).str;
      ++it;
      if (it != et) {
        r += ", ";
      }
    }
    r += "}";
    return ReprResult{r};
  } else if (auto cst_vector = llvm::dyn_cast< llvm::ConstantVector >(cst)) {
    std::string r = "[";
    for (auto it = cst_vector->op_begin(), et = cst_vector->op_end();
         it != et;) {
      r += repr(*it, seen).str;
      ++it;
      if (it != et) {
        r += ", ";
      }
    }
    r += "]";
    return ReprResult{r};
  } else if (auto cst_data_seq =
                 llvm::dyn_cast< llvm::ConstantDataSequential >(cst)) {
    if (cst_data_seq->isString()) {
      return ReprResult{detail::escape_string(cst_data_seq->getAsString())};
    }

    std::string r = "[";
    auto ty = cst_data_seq->getElementType();
    for (unsigned i = 0, n = cst_data_seq->getNumElements(); i < n;) {
      if (ty->isIntegerTy()) {
        r += std::to_string(cst_data_seq->getElementAsInteger(i));
      } else if (ty->isFloatingPointTy()) {
        r += detail::to_string(cst_data_seq->getElementAsAPFloat(i));
      } else {
        ikos_unreachable("unreachable");
      }

      i++;
      if (i != n) {
        r += ", ";
      }
    }
    r += "]";
    return ReprResult{r};
  } else if (auto cst_expr = llvm::dyn_cast< llvm::ConstantExpr >(cst)) {
    auto inst_deleter = [](llvm::Instruction* inst) { inst->deleteValue(); };
    std::unique_ptr< llvm::Instruction, decltype(inst_deleter) >
        inst(cst_expr->getAsInstruction(), inst_deleter);
    return repr(inst.get(), seen);
  } else {
    throw FrontendError("unsupported llvm constant");
  }
}

/// \brief Return the fields of a structure type
std::vector< llvm::DIDerivedType* > struct_fields(llvm::DICompositeType* type) {
  ikos_assert(type != nullptr);
  ikos_assert(type->getTag() == llvm::dwarf::DW_TAG_structure_type ||
              type->getTag() == llvm::dwarf::DW_TAG_class_type);

  if (type->getRawElements() == nullptr) {
    return {}; // e.g, a forward declaration
  }

  std::vector< llvm::DIDerivedType* > fields;

  for (llvm::DINode* member : type->getElements()) {
    if (llvm::isa< llvm::DISubprogram >(member)) {
      // Skip methods
      continue;
    }

    if (llvm::cast< llvm::DIType >(member)->isStaticMember()) {
      // Skip static members
      continue;
    }

    ikos_assert(llvm::isa< llvm::DIDerivedType >(member));
    auto derived_type = llvm::cast< llvm::DIDerivedType >(member);

    if (derived_type->getTag() == llvm::dwarf::DW_TAG_inheritance) {
      // Base class
      ikos_assert(derived_type->getRawBaseType() != nullptr);
      auto base = llvm::cast< llvm::DIType >(derived_type->getRawBaseType());
      base = remove_qualifiers(base);
      ikos_assert(base != nullptr);
      ikos_assert(llvm::isa< llvm::DICompositeType >(base));
      auto parent = llvm::cast< llvm::DICompositeType >(base);
      auto parent_fields = struct_fields(parent);
      fields.insert(fields.end(), parent_fields.begin(), parent_fields.end());
      continue;
    }

    ikos_assert(derived_type->getTag() == llvm::dwarf::DW_TAG_member);
    fields.push_back(derived_type);
  }

  return fields;
}

/// \brief Add a structure field access `addr->field` or `addr.field`
ReprResult add_struct_access(const ReprResult& addr,
                             const llvm::APInt& offset) {
  llvm::DIDerivedType* field = nullptr;

  auto type = remove_qualifiers(addr.pointee_type);
  if (type != nullptr && llvm::isa< llvm::DICompositeType >(type)) {
    auto comp_type = llvm::cast< llvm::DICompositeType >(type);
    if (comp_type->getTag() == llvm::dwarf::DW_TAG_structure_type ||
        comp_type->getTag() == llvm::dwarf::DW_TAG_class_type) {
      std::vector< llvm::DIDerivedType* > fields = struct_fields(comp_type);
      uint64_t index = offset.getZExtValue();

      if (index < fields.size()) {
        field = fields[index];
      }
    }
  }

  std::string s;
  if (is_address(addr.str)) {
    s = "&" + add_deref(addr.str) + ".";
  } else {
    s = "&" + add_parentheses(addr.str) + "->";
  }
  if (field != nullptr) {
    s += field->getName();
    return ReprResult{s, llvm::cast< llvm::DIType >(field->getRawBaseType())};
  } else {
    // Could not guess the field type
    s += to_string(offset);
    return ReprResult{s};
  }
}

/// \brief Add an array access `addr[index]`
ReprResult add_array_access(const ReprResult& addr, const ReprResult& index) {
  llvm::DIType* element_type = nullptr;

  auto type = remove_qualifiers(addr.pointee_type);
  if (type != nullptr) {
    if (auto derived_type = llvm::dyn_cast< llvm::DIDerivedType >(type)) {
      if (derived_type->getTag() == llvm::dwarf::DW_TAG_pointer_type) {
        element_type =
            llvm::cast_or_null< llvm::DIType >(derived_type->getRawBaseType());
      }
    } else if (auto comp_type = llvm::dyn_cast< llvm::DICompositeType >(type)) {
      if (comp_type->getTag() == llvm::dwarf::DW_TAG_array_type) {
        element_type =
            llvm::cast_or_null< llvm::DIType >(comp_type->getRawBaseType());
      }
    }
  }

  std::string s;
  if (is_address(addr.str)) {
    s = "&" + add_deref(addr.str);
  } else {
    s = "&" + add_parentheses(addr.str);
  }
  s += "[" + index.str + "]";
  return ReprResult{s, element_type};
}

/// \brief Return the textual representation of a llvm::Value*
ReprResult repr(llvm::Value* value, ValueSet seen) {
  ikos_assert(value != nullptr);

  // Check for llvm.dbg.value
  if (!llvm::isa< llvm::Constant >(value)) {
    llvm::SmallVector< llvm::DbgValueInst*, 1 > dbg_values;
    llvm::findDbgValues(dbg_values, value);
    auto dbg_value =
        std::find_if(dbg_values.begin(),
                     dbg_values.end(),
                     [](llvm::DbgValueInst* dbg) {
                       return dbg->getExpression()->getNumElements() == 0;
                     });

    if (dbg_value != dbg_values.end()) {
      llvm::DILocalVariable* di_var = (*dbg_value)->getVariable();
      auto di_type = llvm::cast_or_null< llvm::DIType >(di_var->getRawType());
      llvm::StringRef name = di_var->getName();

      if (!name.empty()) {
        return ReprResult{name.str(), pointee_type(di_type)};
      }
    }
  }

  if (auto arg = llvm::dyn_cast< llvm::Argument >(value)) {
    // Check for llvm.dbg.declare and llvm.dbg.addr
    llvm::TinyPtrVector< llvm::DbgVariableIntrinsic* > dbg_addrs =
        llvm::FindDbgAddrUses(arg);
    auto dbg_addr =
        std::find_if(dbg_addrs.begin(),
                     dbg_addrs.end(),
                     [](llvm::DbgVariableIntrinsic* dbg) {
                       return dbg->getExpression()->getNumElements() == 0;
                     });

    if (dbg_addr != dbg_addrs.end()) {
      llvm::DILocalVariable* di_var = (*dbg_addr)->getVariable();
      auto di_type = llvm::cast_or_null< llvm::DIType >(di_var->getRawType());
      llvm::StringRef name = di_var->getName();

      if (!name.empty()) {
        return ReprResult{"&" + demangle(name), di_type};
      }
    }

    return ReprResult{"__arg" + std::to_string(arg->getArgNo())};
  }

  if (auto inline_asm = llvm::dyn_cast< llvm::InlineAsm >(value)) {
    return ReprResult{"asm " +
                      detail::escape_string(inline_asm->getAsmString())};
  }

  if (auto cst = llvm::dyn_cast< llvm::Constant >(value)) {
    return repr(cst, seen);
  }

  if (auto inst = llvm::dyn_cast< llvm::Instruction >(value)) {
    if (auto alloca = llvm::dyn_cast< llvm::AllocaInst >(inst)) {
      // Check for llvm.dbg.declare and llvm.dbg.addr
      llvm::TinyPtrVector< llvm::DbgVariableIntrinsic* > dbg_addrs =
          llvm::FindDbgAddrUses(alloca);
      auto dbg_addr =
          std::find_if(dbg_addrs.begin(),
                       dbg_addrs.end(),
                       [](llvm::DbgVariableIntrinsic* dbg) {
                         return dbg->getExpression()->getNumElements() == 0;
                       });

      if (dbg_addr != dbg_addrs.end()) {
        llvm::DILocalVariable* di_var = (*dbg_addr)->getVariable();
        auto di_type = llvm::cast_or_null< llvm::DIType >(di_var->getRawType());
        llvm::StringRef name = di_var->getName();

        if (!name.empty()) {
          return ReprResult{"&" + demangle(name),
                            alloca->isArrayAllocation() ? pointee_type(di_type)
                                                        : di_type};
        }
      }

      // Last chance, use llvm variable name
      if (alloca->hasName()) {
        return ReprResult{"&" + demangle(alloca->getName())};
      }

      return ReprResult{"&__unnamed_local_var"};
    } else if (auto load = llvm::dyn_cast< llvm::LoadInst >(inst)) {
      ReprResult r = repr(load->getPointerOperand(), seen);
      return ReprResult{detail::add_deref(r.str), pointee_type(r.pointee_type)};
    } else if (auto call = llvm::dyn_cast< llvm::CallInst >(inst)) {
      std::string r =
          detail::add_deref(repr(call->getCalledOperand(), seen).str);
      r += "(";
      for (auto it = call->arg_begin(), et = call->arg_end(); it != et;) {
        r += repr(*it, seen).str;
        ++it;
        if (it != et) {
          r += ", ";
        }
      }
      r += ")";
      return ReprResult{r};
    } else if (auto invoke = llvm::dyn_cast< llvm::InvokeInst >(inst)) {
      std::string r =
          detail::add_deref(repr(invoke->getCalledOperand(), seen).str);
      r += "(";
      for (auto it = invoke->arg_begin(), et = invoke->arg_end(); it != et;) {
        r += repr(*it, seen).str;
        ++it;
        if (it != et) {
          r += ", ";
        }
      }
      r += ")";
      return ReprResult{r};
    } else if (auto cast = llvm::dyn_cast< llvm::CastInst >(inst)) {
      std::string r;

      if (cast->getOpcode() == llvm::Instruction::ZExt ||
          cast->getOpcode() == llvm::Instruction::FPToUI) {
        auto int_type = llvm::cast< llvm::IntegerType >(cast->getType());
        r += "(uint" + std::to_string(int_type->getBitWidth()) + "_t)";
      } else if (cast->getOpcode() == llvm::Instruction::SExt ||
                 cast->getOpcode() == llvm::Instruction::FPToSI) {
        auto int_type = llvm::cast< llvm::IntegerType >(cast->getType());
        r += "(int" + std::to_string(int_type->getBitWidth()) + "_t)";
      } else {
        TypeSet types_seen;
        std::string t = repr(cast->getType(), types_seen);
        r += "(" + t + ")";
      }

      r += detail::add_parentheses(repr(cast->getOperand(0), seen).str);
      return ReprResult{r};
    } else if (auto gep = llvm::dyn_cast< llvm::GetElementPtrInst >(inst)) {
      ReprResult r = repr(gep->getPointerOperand(), seen);

      auto begin = llvm::gep_type_begin(gep);
      auto end = llvm::gep_type_end(gep);

      for (auto it = begin; it != end; ++it) {
        llvm::Value* op = it.getOperand();

        if (it.getStructTypeOrNull() != nullptr) {
          // Shift to get a structure field
          llvm::APInt offset = llvm::cast< llvm::ConstantInt >(op)->getValue();
          r = detail::add_struct_access(r, offset);
        } else if (auto cst = llvm::dyn_cast< llvm::ConstantInt >(op)) {
          // Shift in a sequential type
          if (it == begin && cst->isZero()) {
            continue; // skip
          } else {
            r = detail::add_array_access(r, repr(op, seen));
          }
        } else {
          r = detail::add_array_access(r, repr(op, seen));
        }
      }

      return r;
    } else if (auto bin_op = llvm::dyn_cast< llvm::BinaryOperator >(inst)) {
      std::string r;
      r += detail::add_parentheses(repr(bin_op->getOperand(0), seen).str);
      r += ' ';
      r += detail::repr(bin_op->getOpcode());
      r += ' ';
      r += detail::add_parentheses(repr(bin_op->getOperand(1), seen).str);
      return ReprResult{r};
    } else if (auto cmp = llvm::dyn_cast< llvm::CmpInst >(inst)) {
      std::string r;
      r += detail::add_parentheses(repr(cmp->getOperand(0), seen).str);
      r += ' ';
      r += detail::repr(cmp->getPredicate());
      r += ' ';
      r += detail::add_parentheses(repr(cmp->getOperand(1), seen).str);
      return ReprResult{r};
    } else if (auto phi = llvm::dyn_cast< llvm::PHINode >(inst)) {
      // Avoid infinite recursion
      auto p = seen.insert(phi);
      if (!p.second) {
        return ReprResult{"..."}; // already processing
      }

      // Simple heuristic to detect a ternary `cond ? a : b`
      if (phi->getNumIncomingValues() == 2) {
        auto first = phi->getIncomingBlock(0);
        auto second = phi->getIncomingBlock(1);

        if (first->getUniquePredecessor() != nullptr &&
            second->getUniquePredecessor() != nullptr &&
            first->getUniquePredecessor() == second->getUniquePredecessor()) {
          auto br = llvm::dyn_cast< llvm::BranchInst >(
              first->getUniquePredecessor()->getTerminator());
          if (br != nullptr && br->isConditional()) {
            std::string r;
            r += detail::add_parentheses(repr(br->getCondition(), seen).str);
            r += " ? ";
            r += repr(phi->getIncomingValueForBlock(br->getSuccessor(0)), seen)
                     .str;
            r += " : ";
            r += repr(phi->getIncomingValueForBlock(br->getSuccessor(1)), seen)
                     .str;
            return ReprResult{r};
          }
        }
      }

      // Else
      std::string r = "{";
      for (unsigned i = 0, n = phi->getNumIncomingValues(); i < n;) {
        llvm::BasicBlock* bb = phi->getIncomingBlock(i);
        if (bb->hasName()) {
          r += bb->getName();
        } else {
          r += std::to_string(i);
        }
        r += ": ";
        r += repr(phi->getIncomingValue(i), seen).str;
        i++;
        if (i != n) {
          r += ", ";
        }
      }
      r += "}";
      return ReprResult{r};
    } else if (auto extractvalue =
                   llvm::dyn_cast< llvm::ExtractValueInst >(inst)) {
      std::string r;
      r += detail::add_parentheses(
          repr(extractvalue->getAggregateOperand(), seen).str);
      for (auto it = extractvalue->idx_begin(), et = extractvalue->idx_end();
           it != et;
           ++it) {
        r += '.';
        r += std::to_string(*it);
      }
      return ReprResult{r};
    } else if (auto insertvalue =
                   llvm::dyn_cast< llvm::InsertValueInst >(inst)) {
      std::string r;
      r += detail::add_parentheses(
          repr(insertvalue->getAggregateOperand(), seen).str);
      r += '[';
      for (auto it = insertvalue->idx_begin(), et = insertvalue->idx_end();
           it != et;) {
        r += std::to_string(*it);
        ++it;
        if (it != et) {
          r += '.';
        }
      }
      r += ": ";
      r += repr(insertvalue->getInsertedValueOperand(), seen).str;
      r += ']';
      return ReprResult{r};
    } else if (llvm::isa< llvm::LandingPadInst >(inst)) {
      return ReprResult{"__current_exception"};
    } else {
      std::ostringstream buf;
      buf << "unsupported llvm instruction: " << inst->getOpcodeName();
      throw FrontendError(buf.str());
    }
  }

  throw FrontendError("unsupported llvm value");
}

/// \brief ar::Value visitor returning a textual representation
struct OperandReprVisitor {
  using ResultType = std::string;

  std::string operator()(ar::UndefinedConstant*) const { return "undefined"; }

  std::string operator()(ar::IntegerConstant* c) const {
    return c->value().str();
  }

  std::string operator()(ar::FloatConstant* c) const { return c->value(); }

  std::string operator()(ar::NullConstant*) const { return "NULL"; }

  std::string operator()(ar::StructConstant* c) const {
    std::string r = "{";
    for (auto it = c->field_begin(), et = c->field_end(); it != et;) {
      r += ar::apply_visitor(*this, it->value);
      ++it;
      if (it != et) {
        r += ", ";
      }
    }
    r += "}";
    return r;
  }

  std::string operator()(ar::ArrayConstant* c) const {
    std::string r = "[";
    for (auto it = c->element_begin(), et = c->element_end(); it != et;) {
      r += ar::apply_visitor(*this, *it);
      ++it;
      if (it != et) {
        r += ", ";
      }
    }
    r += "]";
    return r;
  }

  std::string operator()(ar::VectorConstant* c) const {
    std::string r = "[";
    for (auto it = c->element_begin(), et = c->element_end(); it != et;) {
      r += ar::apply_visitor(*this, *it);
      ++it;
      if (it != et) {
        r += ", ";
      }
    }
    r += "]";
    return r;
  }

  std::string operator()(ar::AggregateZeroConstant*) const { return "{0}"; }

  std::string operator()(ar::FunctionPointerConstant* c) const {
    ar::Function* fun = c->function();
    return "&" + demangle(FunctionsTable::name(fun));
  }

  std::string operator()(ar::InlineAssemblyConstant* c) const {
    return "asm " + escape_string(c->code());
  }

  std::string operator()(ar::GlobalVariable* gv) const {
    ikos_assert(gv->has_frontend());
    auto llvm_gv = gv->frontend< llvm::GlobalVariable >();

    // Check for debug info
    llvm::SmallVector< llvm::DIGlobalVariableExpression*, 1 > dbgs;
    llvm_gv->getDebugInfo(dbgs);

    if (!dbgs.empty()) {
      llvm::DIGlobalVariable* di_gv = dbgs[0]->getVariable();
      llvm::StringRef name = di_gv->getName();

      if (!name.empty()) {
        return "&" + demangle(name);
      }
    }

    // If it's a constant (e.g, a string)
    if (llvm_gv->isConstant() && llvm_gv->hasInitializer()) {
      return "&" + OperandsTable::repr(llvm_gv->getInitializer());
    }

    // Last chance, use llvm variable name
    if (llvm_gv->hasName()) {
      return "&" + demangle(llvm_gv->getName());
    }

    return "&__unnamed_global_var";
  }

  std::string operator()(ar::LocalVariable* lv) const {
    ikos_assert(lv->has_frontend());
    auto value = lv->frontend< llvm::Value >();
    ikos_assert(llvm::isa< llvm::AllocaInst >(value));
    auto alloca = llvm::cast< llvm::AllocaInst >(value);

    // Check for llvm.dbg.declare and llvm.dbg.addr
    llvm::TinyPtrVector< llvm::DbgVariableIntrinsic* > dbg_addrs =
        llvm::FindDbgAddrUses(alloca);
    auto dbg_addr =
        std::find_if(dbg_addrs.begin(),
                     dbg_addrs.end(),
                     [](llvm::DbgVariableIntrinsic* dbg) {
                       return dbg->getExpression()->getNumElements() == 0;
                     });

    if (dbg_addr != dbg_addrs.end()) {
      llvm::DILocalVariable* di_var = (*dbg_addr)->getVariable();
      llvm::StringRef name = di_var->getName();

      if (!name.empty()) {
        return "&" + demangle(name);
      }
    }

    // Check for llvm.dbg.value
    llvm::SmallVector< llvm::DbgValueInst*, 1 > dbg_values;
    llvm::findDbgValues(dbg_values, alloca);
    auto dbg_value =
        std::find_if(dbg_values.begin(),
                     dbg_values.end(),
                     [](llvm::DbgValueInst* dbg) {
                       return dbg->getExpression()->getNumElements() == 0;
                     });

    if (dbg_value != dbg_values.end()) {
      llvm::DILocalVariable* di_var = (*dbg_value)->getVariable();
      llvm::StringRef name = di_var->getName();

      if (!name.empty()) {
        return "&" + demangle(name);
      }
    }

    // Last chance, use llvm variable name
    if (alloca->hasName()) {
      return "&" + demangle(alloca->getName());
    }

    return "&__unnamed_local_var";
  }

  std::string operator()(ar::InternalVariable* iv) const {
    ikos_assert(iv->has_frontend());
    auto value = iv->frontend< llvm::Value >();
    return OperandsTable::repr(value);
  }

}; // end struct OperandReprVisitor

} // end anonymous namespace
} // end namespace detail

std::string OperandsTable::repr(llvm::Type* type) {
  return detail::repr(type, detail::TypeSet{});
}

std::string OperandsTable::repr(llvm::Constant* cst) {
  return detail::repr(cst, detail::ValueSet{}).str;
}

std::string OperandsTable::repr(llvm::Value* value) {
  return detail::repr(value, detail::ValueSet{}).str;
}

std::string OperandsTable::repr(ar::Value* value) {
  return ar::apply_visitor(detail::OperandReprVisitor{}, value);
}

} // end namespace analyzer
} // end namespace ikos
