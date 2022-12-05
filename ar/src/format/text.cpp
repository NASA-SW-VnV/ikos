/*******************************************************************************
 *
 * \file
 * \brief Text format for the abstract representation, implementation
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

#include <boost/container/flat_set.hpp>

#include <ikos/ar/format/text.hpp>
#include <ikos/ar/semantic/statement_visitor.hpp>
#include <ikos/ar/semantic/type_visitor.hpp>
#include <ikos/ar/semantic/value_visitor.hpp>
#include <ikos/ar/support/assert.hpp>

namespace ikos {
namespace ar {

void TextFormatter::format(std::ostream& o, Bundle* bundle) const {
  o << "// Bundle\n";

  // data layout
  const DataLayout& data_layout = bundle->data_layout();

  // endianness
  o << "target-endianness = ";
  if (data_layout.is_little_endian()) {
    o << "little-endian\n";
  } else {
    o << "big-endian\n";
  }

  // size of pointers
  o << "target-pointer-size = " << data_layout.pointers.bit_width << " bits\n";

  // target triple
  if (!bundle->target_triple().empty()) {
    o << "target-triple = " << bundle->target_triple() << "\n";
  }

  if (!this->order_globals()) {
    // global variables
    for (auto it = bundle->global_begin(), et = bundle->global_end(); it != et;
         ++it) {
      o << "\n";
      this->format(o, *it);
    }

    // functions
    for (auto it = bundle->function_begin(), et = bundle->function_end();
         it != et;
         ++it) {
      o << "\n";
      this->format(o, *it);
    }
  } else {
    // sort global variables and functions by name, before formatting
    std::vector< GlobalVariable* > globals(bundle->global_begin(),
                                           bundle->global_end());
    std::vector< Function* > functions(bundle->function_begin(),
                                       bundle->function_end());

    std::sort(globals.begin(),
              globals.end(),
              [](GlobalVariable* a, GlobalVariable* b) {
                return a->name() < b->name();
              });
    std::sort(functions.begin(), functions.end(), [](Function* a, Function* b) {
      return a->name() < b->name();
    });

    // format globals and functions
    for (GlobalVariable* gv : globals) {
      o << "\n";
      this->format(o, gv);
    }
    for (Function* fun : functions) {
      o << "\n";
      this->format(o, fun);
    }
  }
}

void TextFormatter::format(std::ostream& o, GlobalVariable* gv) const {
  // declare/define
  if (gv->is_declaration()) {
    o << "declare ";
  } else {
    o << "define ";
  }

  // type and name
  this->format(o, gv->type());
  o << " @" << gv->name();

  // alignment
  if (gv->has_alignment()) {
    o << ", align " << gv->alignment();
  }

  // initializer
  if (gv->is_declaration()) {
    o << "\n";
  } else {
    o << ", init {\n";
    this->format(o, gv->initializer());
    o << "}\n";
  }
}

void TextFormatter::format_header(std::ostream& o,
                                  const Function* f,
                                  Namer& namer) const {
  FunctionType* type = f->type(); // declare/define
  if (f->is_declaration()) {
    o << "declare ";
  } else {
    namer.init(f->body()); // initialize the namer
    o << "define ";
  }

  // return type and name
  format(o, type->return_type());
  o << " @" << f->name();

  // parameters
  o << "(";
  if (f->is_declaration()) {
    for (auto it = type->param_begin(), et = type->param_end(); it != et;) {
      format(o, *it);
      ++it;
      if (it != et) {
        o << ", ";
      }
    }
  } else {
    for (auto it = f->param_begin(), et = f->param_end(); it != et;) {
      format(o, *it, namer, true);
      ++it;
      if (it != et) {
        o << ", ";
      }
    }
  }
  if (f->is_var_arg()) {
    if (type->num_parameters() > 0) {
      o << ", ";
    }
    o << "...";
  }
  o << ")";
}

void TextFormatter::format(std::ostream& o, Function* f) const {
  Namer namer;
  this->format_header(o, f, namer);

  // body
  if (f->is_declaration()) {
    o << "\n";
  } else {
    o << " {\n";
    this->format(o, f->body(), namer);
    o << "}\n";
  }
}

void TextFormatter::format(std::ostream& o, Code* code) const {
  this->format(o, code, Namer(code));
}

void TextFormatter::format(std::ostream& o,
                           Code* code,
                           const Namer& namer) const {
  for (auto it = code->begin(), et = code->end(); it != et; ++it) {
    this->format(o, *it, namer);
  }
}

void TextFormatter::format(std::ostream& o, BasicBlock* bb) const {
  this->format(o, bb, Namer(bb->code()));
}

void TextFormatter::format(std::ostream& o,
                           BasicBlock* bb,
                           const Namer& namer) const {
  Code* code = bb->code();

  // name
  o << "#" << namer.name(bb);

  // attributes
  if (code->entry_block() == bb) {
    o << " !entry";
  }
  if (code->exit_block_or_null() == bb) {
    o << " !exit";
  }

  // predecessors
  if (bb->num_predecessors() > 0) {
    o << " predecessors={";
    for (auto it = bb->predecessor_begin(), et = bb->predecessor_end();
         it != et;) {
      o << "#" << namer.name(*it);
      ++it;
      if (it != et) {
        o << ", ";
      }
    }
    o << "}";
  }

  // successors
  if (bb->num_successors() > 0) {
    o << " successors={";
    for (auto it = bb->successor_begin(), et = bb->successor_end(); it != et;) {
      o << "#" << namer.name(*it);
      ++it;
      if (it != et) {
        o << ", ";
      }
    }
    o << "}";
  }

  // statements
  o << " {\n";
  for (auto it = bb->begin(), et = bb->end(); it != et; ++it) {
    o << "  ";
    this->format(o, *it, namer);
    o << "\n";
  }
  o << "}\n";
}

void TextFormatter::format(std::ostream& o, Statement* stmt) const {
  Namer namer(stmt->code());
  this->format(o, stmt, namer);
}

namespace {

/// \brief Visitor to format a statement into text format
class FormatTextStatement {
public:
  using ResultType = void;

public:
  const TextFormatter& formatter;
  std::ostream& o;
  const Namer& namer;

public:
  FormatTextStatement(const TextFormatter& formatter_,
                      std::ostream& o_,
                      const Namer& namer_)
      : formatter(formatter_), o(o_), namer(namer_) {}

  void operator()(Assignment* s) {
    formatter.format(o, s->result(), namer, formatter.show_result_type());
    o << " = ";
    formatter.format(o, s->operand(), namer, formatter.show_operand_types());
  }

  void operator()(UnaryOperation* s) {
    formatter.format(o, s->result(), namer, formatter.show_result_type());
    o << " = " << UnaryOperation::operator_text(s->op()) << " ";
    formatter.format(o, s->operand(), namer, formatter.show_operand_types());
  }

  void operator()(BinaryOperation* s) {
    formatter.format(o, s->result(), namer, formatter.show_result_type());
    o << " = ";
    formatter.format(o, s->left(), namer, formatter.show_operand_types());
    o << " ";
    o << BinaryOperation::operator_text(s->op());
    if (s->has_no_wrap()) {
      o << ".nw";
    }
    if (s->is_exact()) {
      o << ".exact";
    }
    o << " ";
    formatter.format(o, s->right(), namer, formatter.show_operand_types());
  }

  void operator()(Comparison* s) {
    formatter.format(o, s->left(), namer, formatter.show_operand_types());
    o << " " << Comparison::predicate_text(s->predicate()) << " ";
    formatter.format(o, s->right(), namer, formatter.show_operand_types());
  }

  void operator()(ReturnValue* s) {
    if (s->has_operand()) {
      o << "return ";
      formatter.format(o, s->operand(), namer, formatter.show_operand_types());
    } else {
      o << "return";
    }
  }

  void operator()(Unreachable* /*s*/) { o << "unreachable"; }

  void operator()(Allocate* s) {
    formatter.format(o, s->result(), namer, formatter.show_result_type());
    o << " = allocate ";
    formatter.format(o, s->allocated_type());
    o << ", ";
    formatter.format(o, s->array_size(), namer, formatter.show_operand_types());
    if (s->result()->has_alignment()) {
      o << ", align " << s->result()->alignment();
    }
  }

  void operator()(PointerShift* s) {
    formatter.format(o, s->result(), namer, formatter.show_result_type());
    o << " = ptrshift ";
    formatter.format(o, s->pointer(), namer, formatter.show_operand_types());

    // terms
    for (auto it = s->term_begin(), et = s->term_end(); it != et; ++it) {
      auto term = *it;
      o << ", " << term.first << " * ";
      formatter.format(o, term.second, namer, formatter.show_operand_types());
    }
  }

  void operator()(Load* s) {
    formatter.format(o, s->result(), namer, formatter.show_result_type());
    o << " = load ";
    if (s->is_volatile()) {
      o << "volatile ";
    }
    formatter.format(o, s->operand(), namer, formatter.show_operand_types());
    if (s->has_alignment()) {
      o << ", align " << s->alignment();
    }
  }

  void operator()(Store* s) {
    o << "store ";
    if (s->is_volatile()) {
      o << "volatile ";
    }
    formatter.format(o, s->pointer(), namer, formatter.show_operand_types());
    o << ", ";
    formatter.format(o, s->value(), namer, formatter.show_operand_types());
    if (s->has_alignment()) {
      o << ", align " << s->alignment();
    }
  }

  void operator()(ExtractElement* s) {
    formatter.format(o, s->result(), namer, formatter.show_result_type());
    o << " = extractelement ";
    formatter.format(o, s->aggregate(), namer, formatter.show_operand_types());
    o << ", ";
    formatter.format(o, s->offset(), namer, formatter.show_operand_types());
  }

  void operator()(InsertElement* s) {
    formatter.format(o, s->result(), namer, formatter.show_result_type());
    o << " = insertelement ";
    formatter.format(o, s->aggregate(), namer, formatter.show_operand_types());
    o << ", ";
    formatter.format(o, s->offset(), namer, formatter.show_operand_types());
    o << ", ";
    formatter.format(o, s->element(), namer, formatter.show_operand_types());
  }

  void operator()(ShuffleVector* s) {
    formatter.format(o, s->result(), namer, formatter.show_result_type());
    o << " = shufflevector ";
    formatter.format(o, s->left(), namer, formatter.show_operand_types());
    o << ", ";
    formatter.format(o, s->right(), namer, formatter.show_operand_types());
  }

  void operator()(Call* s) {
    // result
    if (s->has_result()) {
      formatter.format(o, s->result(), namer, formatter.show_result_type());
      o << " = ";
    }

    // op
    o << "call ";

    // called
    formatter.format(o, s->called(), namer, formatter.show_operand_types());

    // arguments
    o << "(";
    for (auto it = s->arg_begin(), et = s->arg_end(); it != et;) {
      formatter.format(o, *it, namer, formatter.show_operand_types());
      ++it;
      if (it != et) {
        o << ", ";
      }
    }
    o << ")";
  }

  void operator()(Invoke* s) {
    // result
    if (s->has_result()) {
      formatter.format(o, s->result(), namer, formatter.show_result_type());
      o << " = ";
    }

    // op
    o << "invoke ";

    // called
    formatter.format(o, s->called(), namer, formatter.show_operand_types());

    // arguments
    o << "(";
    for (auto it = s->arg_begin(), et = s->arg_end(); it != et;) {
      formatter.format(o, *it, namer, formatter.show_operand_types());
      ++it;
      if (it != et) {
        o << ", ";
      }
    }
    o << ")";

    // normal/exception
    o << " normal=#" << namer.name(s->normal_dest());
    o << " exc=#" << namer.name(s->exception_dest());
  }

  void operator()(LandingPad* s) {
    formatter.format(o, s->result(), namer, formatter.show_result_type());
    o << " = landingpad";
  }

  void operator()(Resume* s) {
    o << "resume ";
    formatter.format(o, s->operand(), namer, formatter.show_operand_types());
  }

}; // end class FormatTextStatement

} // end anonymous namespace

void TextFormatter::format(std::ostream& o,
                           Statement* stmt,
                           const Namer& namer) const {
  FormatTextStatement vis(*this, o, namer);
  apply_visitor(vis, stmt);
}

namespace {

/// \brief Set of types
using TypeSet = boost::container::flat_set< Type* >;

/// \brief Visitor to format a type into text format
class FormatTextType {
public:
  using ResultType = void;

public:
  std::ostream& o;
  TypeSet seen;

public:
  explicit FormatTextType(std::ostream& o_) : o(o_) {}

  FormatTextType(std::ostream& o_, TypeSet seen_)
      : o(o_), seen(std::move(seen_)) {}

  void operator()(VoidType* /*t*/) { o << "void"; }

  void operator()(IntegerType* t) {
    if (t->is_unsigned()) {
      o << "ui";
    } else {
      o << "si";
    }
    o << t->bit_width();
  }

  void operator()(FloatType* t) {
    switch (t->float_semantic()) {
      case Half:
        o << "half";
        break;
      case Float:
        o << "float";
        break;
      case Double:
        o << "double";
        break;
      case X86_FP80:
        o << "x86_fp80";
        break;
      case FP128:
        o << "fp128";
        break;
      case PPC_FP128:
        o << "ppc_fp128";
        break;
      default:
        ikos_unreachable("unknown float semantic");
    }
  }

  void operator()(PointerType* t) {
    apply_visitor(*this, t->pointee());
    o << "*";
  }

  void operator()(StructType* t) {
    if (t->packed()) {
      o << "<";
    }
    o << "{";
    if (seen.find(t) != seen.end()) {
      o << "...";
    } else {
      seen.insert(t);
      for (auto it = t->field_begin(), et = t->field_end(); it != et;) {
        o << it->offset << ": ";
        FormatTextType field_visitor(o, seen);
        apply_visitor(field_visitor, it->type);
        ++it;
        if (it != et) {
          o << ", ";
        }
      }
    }
    o << "}";
    if (t->packed()) {
      o << ">";
    }
  }

  void operator()(ArrayType* t) {
    o << "[" << t->num_elements() << " x ";
    apply_visitor(*this, t->element_type());
    o << "]";
  }

  void operator()(VectorType* t) {
    o << "<" << t->num_elements() << " x ";
    apply_visitor(*this, t->element_type());
    o << ">";
  }

  void operator()(OpaqueType* /*t*/) { o << "opaque"; }

  void operator()(FunctionType* t) {
    apply_visitor(*this, t->return_type());
    o << " (";
    for (auto it = t->param_begin(), et = t->param_end(); it != et;) {
      apply_visitor(*this, *it);
      ++it;
      if (it != et) {
        o << ", ";
      }
    }
    if (t->is_var_arg()) {
      if (t->num_parameters() > 0) {
        o << ", ";
      }
      o << "...";
    }
    o << ")";
  }

}; // end class FormatTextType

} // end anonymous namespace

void TextFormatter::format(std::ostream& o, Type* type) const {
  FormatTextType vis(o);
  apply_visitor(vis, type);
}

namespace {

/// \brief Visitor to format a value into text format
class FormatTextValue {
public:
  using ResultType = void;

public:
  std::ostream& o;
  const Namer& namer;

public:
  FormatTextValue(std::ostream& o_, const Namer& namer_)
      : o(o_), namer(namer_) {}

  void operator()(UndefinedConstant* /*c*/) { o << "undef"; }

  void operator()(IntegerConstant* c) { o << c->value(); }

  void operator()(FloatConstant* c) { o << c->value(); }

  void operator()(NullConstant* /*c*/) { o << "null"; }

  void operator()(StructConstant* c) {
    o << "{";
    for (auto it = c->field_begin(), et = c->field_end(); it != et;) {
      o << it->offset << ": ";
      apply_visitor(*this, it->value);
      ++it;
      if (it != et) {
        o << ", ";
      }
    }
    o << "}";
  }

  void operator()(ArrayConstant* c) {
    o << "[";
    for (auto it = c->element_begin(), et = c->element_end(); it != et;) {
      apply_visitor(*this, *it);
      ++it;
      if (it != et) {
        o << ", ";
      }
    }
    o << "]";
  }

  void operator()(VectorConstant* c) {
    o << "<";
    for (auto it = c->element_begin(), et = c->element_end(); it != et;) {
      apply_visitor(*this, *it);
      ++it;
      if (it != et) {
        o << ", ";
      }
    }
    o << ">";
  }

  void operator()(AggregateZeroConstant* /*c*/) { o << "aggregate_zero"; }

  void operator()(FunctionPointerConstant* c) {
    o << "@" << c->function()->name();
  }

  void operator()(InlineAssemblyConstant* c) {
    o << "asm \"" << c->code() << "\"";
  }

  void operator()(GlobalVariable* v) { o << "@" << v->name(); }

  void operator()(LocalVariable* v) { o << "$" << namer.name(v); }

  void operator()(InternalVariable* v) { o << "%" << namer.name(v); }

}; // end class FormatTextValue

} // end anonymous namespace

void TextFormatter::format(std::ostream& o,
                           Value* value,
                           const Namer& namer,
                           bool show_type) const {
  if (show_type) {
    this->format(o, value->type());
    o << " ";
  }
  FormatTextValue vis(o, namer);
  apply_visitor(vis, value);
}

} // end namespace ar
} // end namespace ikos
