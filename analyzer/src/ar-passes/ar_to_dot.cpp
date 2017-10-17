/*******************************************************************************
 *
 * ARBOS pass that translates AR code into dot format
 *
 * Authors: Jorge A. Navas
 *
 * Contributors: Clement Decoodt
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
#include <set>
#include <sstream>

#include <analyzer/ar-wrapper/wrapper.hpp>

namespace arbos {
namespace passes {

class ARToDotVisitor : public arbos_visitor_api {
public:
  typedef std::shared_ptr< ARToDotVisitor > ARToDotVisitorPtr;

private:
  std::string _cfg_dot;
  std::string TAB_SP;
  std::string NEWLINE;
  std::string SINGLE_SP;

  // DO NOT IMPLEMENT
  ARToDotVisitor(const ARToDotVisitor&);
  ARToDotVisitor& operator=(const ARToDotVisitor&);

public:
  ARToDotVisitor() : _cfg_dot(""), TAB_SP(""), NEWLINE("\\l"), SINGLE_SP(" ") {}

  std::string cfg_dot() const { return _cfg_dot; }

  void visit_start(Basic_Block_ref b) {
    _cfg_dot += "\t" + armour(ar::getName(b)) + " [shape=record,label=\"{" +
                ar::getName(b) + ":" + NEWLINE;
  }

  void visit_end(Basic_Block_ref b) {
    _cfg_dot += "}\"]\n";
    BBRange ids = ar::getSuccs(b);
    for (BBRange::iterator p = ids.begin(), pe = ids.end(); p != pe; p++) {
      _cfg_dot += "\t" + armour(ar::getName(b)) + " -> " +
                  armour(ar::getName(*p)) + ";\n";
    }
  }

  void visit(Arith_Op_ref stmt) {
    _cfg_dot += TAB_SP;
    toDot(ar::getResult(stmt));
    _cfg_dot += SINGLE_SP;
    _cfg_dot += "=";
    _cfg_dot += SINGLE_SP;
    toDot(ar::getArithOp(stmt));
    _cfg_dot += "(";
    toDot(ar::getLeftOp(stmt));
    _cfg_dot += ", ";
    toDot(ar::getRightOp(stmt));
    _cfg_dot += ")";
    _cfg_dot += NEWLINE;
  }

  void visit(FP_Op_ref stmt) {
    _cfg_dot += TAB_SP;
    toDot(ar::getResult(stmt));
    _cfg_dot += SINGLE_SP;
    _cfg_dot += "=";
    _cfg_dot += SINGLE_SP;
    toDot(ar::getFPOp(stmt));
    _cfg_dot += "(";
    toDot(ar::getLeftOp(stmt));
    _cfg_dot += ", ";
    toDot(ar::getRightOp(stmt));
    _cfg_dot += ")";
    _cfg_dot += NEWLINE;
  }

  void visit(Integer_Comparison_ref stmt) {
    _cfg_dot += TAB_SP;
    toDot(ar::getLeftOp(stmt));
    _cfg_dot += SINGLE_SP;
    if (ar::isPositive(stmt)) {
      toDot(to_str(ar::getPredicate(stmt)));
    } else {
      toDot(negatePredicate(ar::getPredicate(stmt)));
    }
    _cfg_dot += SINGLE_SP;
    toDot(ar::getRightOp(stmt));
    _cfg_dot += NEWLINE;
  }

  void visit(FP_Comparison_ref stmt) {
    _cfg_dot += TAB_SP;
    toDot(ar::getLeftOp(stmt));
    _cfg_dot += SINGLE_SP;
    if (ar::isPositive(stmt)) {
      toDot(to_str(ar::getPredicate(stmt)));
    } else {
      toDot(negatePredicate(ar::getPredicate(stmt)));
    }
    _cfg_dot += SINGLE_SP;
    toDot(ar::getRightOp(stmt));
    _cfg_dot += NEWLINE;
  }

  void visit(Assignment_ref stmt) {
    _cfg_dot += TAB_SP;
    toDot(ar::getLeftOp(stmt));
    _cfg_dot += SINGLE_SP;
    _cfg_dot += "=";
    _cfg_dot += SINGLE_SP;
    toDot(ar::getRightOp(stmt));
    _cfg_dot += NEWLINE;
  }

  void visit(Conv_Op_ref stmt) {
    _cfg_dot += TAB_SP;
    toDot(ar::getLeftOp(stmt));
    _cfg_dot += SINGLE_SP;
    _cfg_dot += "=";
    _cfg_dot += SINGLE_SP;
    toDot(ar::getConvOp(stmt));
    _cfg_dot += SINGLE_SP;
    toDot(ar::getRightOp(stmt));
    _cfg_dot += NEWLINE;
  }

  void visit(Bitwise_Op_ref stmt) {
    _cfg_dot += TAB_SP;
    toDot(ar::getResult(stmt));
    _cfg_dot += SINGLE_SP;
    _cfg_dot += "=";
    _cfg_dot += SINGLE_SP;
    toDot(ar::getBitwiseOp(stmt));
    _cfg_dot += "(";
    toDot(ar::getLeftOp(stmt));
    _cfg_dot += ", ";
    toDot(ar::getRightOp(stmt));
    _cfg_dot += ")";
    _cfg_dot += NEWLINE;
  }

  void visit(Allocate_ref stmt) {
    _cfg_dot += TAB_SP;
    toDot(ar::getResult(stmt));
    _cfg_dot += SINGLE_SP;
    _cfg_dot += "=";
    _cfg_dot += SINGLE_SP;
    _cfg_dot += "allocate(";
    toDot(ar::getArraySize(stmt));
    _cfg_dot += ", ";
    toDot(ar::getAllocatedType(stmt));
    _cfg_dot += ")";
    _cfg_dot += NEWLINE;
  }

  void visit(Store_ref stmt) {
    _cfg_dot += TAB_SP;
    _cfg_dot += "memory[";
    toDot(ar::getPointer(stmt));
    _cfg_dot += "]";
    _cfg_dot += SINGLE_SP;
    _cfg_dot += "=";
    _cfg_dot += SINGLE_SP;
    toDot(ar::getValue(stmt));
    alignToDot(ar::getAlignment(stmt));
    _cfg_dot += NEWLINE;
  }

  void visit(Load_ref stmt) {
    _cfg_dot += TAB_SP;
    toDot(ar::getResult(stmt));
    _cfg_dot += SINGLE_SP;
    _cfg_dot += "=";
    _cfg_dot += SINGLE_SP;
    _cfg_dot += "memory[";
    toDot(ar::getPointer(stmt));
    _cfg_dot += "]";
    alignToDot(ar::getAlignment(stmt));
    _cfg_dot += NEWLINE;
  }

  void visit(Insert_Element_ref stmt) {
    _cfg_dot += TAB_SP;
    toDot(ar::getResult(stmt));
    _cfg_dot += SINGLE_SP;
    _cfg_dot += "=";
    _cfg_dot += SINGLE_SP;
    _cfg_dot += "insert_elem(";
    toDot(ar::getAggregate(stmt));
    _cfg_dot += ", ";
    toDot(ar::getElement(stmt));
    _cfg_dot += ", ";
    toDot(ar::getOffset(stmt));
    _cfg_dot += ")";
    _cfg_dot += NEWLINE;
  }

  void visit(Extract_Element_ref stmt) {
    _cfg_dot += TAB_SP;
    toDot(ar::getResult(stmt));
    _cfg_dot += SINGLE_SP;
    _cfg_dot += "=";
    _cfg_dot += SINGLE_SP;
    _cfg_dot += "extract_elem(";
    toDot(ar::getAggregate(stmt));
    _cfg_dot += ", ";
    toDot(ar::getOffset(stmt));
    _cfg_dot += ")";
    _cfg_dot += NEWLINE;
  }

  void visit(Pointer_Shift_ref stmt) {
    _cfg_dot += TAB_SP;
    toDot(ar::getResult(stmt));
    _cfg_dot += SINGLE_SP;
    _cfg_dot += "=";
    _cfg_dot += SINGLE_SP;
    _cfg_dot += "ptr_shift(";
    toDot(ar::getBase(stmt));
    _cfg_dot += ", ";
    toDot(ar::getOffset(stmt));
    _cfg_dot += ")";
    _cfg_dot += NEWLINE;
  }

  void visit(Abstract_Variable_ref stmt) {
    _cfg_dot += TAB_SP;
    _cfg_dot += "havoc(";
    toDot(ar::getVar(stmt));
    _cfg_dot += ")";
    _cfg_dot += NEWLINE;
  }

  void visit(Abstract_Memory_ref stmt) {
    _cfg_dot += TAB_SP;
    _cfg_dot += "havoc(";
    toDot(ar::getPointer(stmt));
    _cfg_dot += ", ";
    toDot(ar::getLen(stmt));
    _cfg_dot += ")";
    _cfg_dot += NEWLINE;
  }

  void visit(MemCpy_ref stmt) {
    _cfg_dot += TAB_SP;
    _cfg_dot += "memcpy(";
    toDot(ar::getTarget(stmt));
    _cfg_dot += ", ";
    toDot(ar::getSource(stmt));
    _cfg_dot += ", ";
    toDot(ar::getLen(stmt));
    alignToDot(ar::getAlignment(stmt));
    _cfg_dot += ")";
    _cfg_dot += NEWLINE;
  }

  void visit(MemMove_ref stmt) {
    _cfg_dot += TAB_SP;
    _cfg_dot += "memmove(";
    toDot(ar::getTarget(stmt));
    _cfg_dot += ", ";
    toDot(ar::getSource(stmt));
    _cfg_dot += ", ";
    toDot(ar::getLen(stmt));
    alignToDot(ar::getAlignment(stmt));
    _cfg_dot += ")";
    _cfg_dot += NEWLINE;
  }

  void visit(MemSet_ref stmt) {
    _cfg_dot += TAB_SP;
    _cfg_dot += "memset(";
    toDot(ar::getBase(stmt));
    _cfg_dot += ", ";
    toDot(ar::getValue(stmt));
    _cfg_dot += ", ";
    toDot(ar::getLen(stmt));
    alignToDot(ar::getAlignment(stmt));
    _cfg_dot += ")";
    _cfg_dot += NEWLINE;
  }

  void visit(Call_ref call) {
    _cfg_dot += TAB_SP;
    boost::optional< Internal_Variable_ref > lhs = ar::getReturnValue(call);
    if (lhs) {
      toDot(*lhs);
      _cfg_dot += SINGLE_SP;
      _cfg_dot += "=";
      _cfg_dot += SINGLE_SP;
    }

    _cfg_dot += "call ";

    if (ar::isIndirectCall(call)) {
      _cfg_dot += ar::getName(ar::getIndirectCallVar(call));
    } else {
      _cfg_dot += ar::getFunctionName(call);
    }

    toDot(ar::getArguments(call));
    _cfg_dot += NEWLINE;
  }

  void visit(Invoke_ref stmt) {
    Call_ref call = ar::getFunctionCall(stmt);

    _cfg_dot += TAB_SP;
    boost::optional< Internal_Variable_ref > lhs = ar::getReturnValue(call);
    if (lhs) {
      toDot(*lhs);
      _cfg_dot += SINGLE_SP;
      _cfg_dot += "=";
      _cfg_dot += SINGLE_SP;
    }

    _cfg_dot += "invoke ";

    if (ar::isIndirectCall(call)) {
      _cfg_dot += ar::getName(ar::getIndirectCallVar(call));
    } else {
      _cfg_dot += ar::getFunctionName(call);
    }

    toDot(ar::getArguments(call));
    _cfg_dot += NEWLINE;
  }

  void visit(Return_Value_ref stmt) {
    _cfg_dot += TAB_SP;
    _cfg_dot += "return";
    _cfg_dot += SINGLE_SP;
    boost::optional< Operand_ref > res = ar::getReturnValue(stmt);
    if (res)
      toDot(*res);
    _cfg_dot += NEWLINE;
  }

  void visit(Landing_Pad_ref stmt) {
    _cfg_dot += TAB_SP;
    _cfg_dot += "landingpad(";
    toDot(ar::getVar(stmt));
    _cfg_dot += ")";
    _cfg_dot += NEWLINE;
  }

  void visit(Resume_ref stmt) {
    _cfg_dot += TAB_SP;
    _cfg_dot += "resume(";
    toDot(ar::getVar(stmt));
    _cfg_dot += ")";
    _cfg_dot += NEWLINE;
  }

  void visit(Unreachable_ref) {
    _cfg_dot += TAB_SP;
    _cfg_dot += "unreachable";
    _cfg_dot += NEWLINE;
  }

  void visit(VA_Start_ref stmt) {
    _cfg_dot += TAB_SP;
    _cfg_dot += "va_start(";
    toDot(ar::getPointer(stmt));
    _cfg_dot += ")";
    _cfg_dot += NEWLINE;
  }

  void visit(VA_End_ref stmt) {
    _cfg_dot += TAB_SP;
    _cfg_dot += "va_end(";
    toDot(ar::getPointer(stmt));
    _cfg_dot += ")";
    _cfg_dot += NEWLINE;
  }

  void visit(VA_Arg_ref stmt) {
    _cfg_dot += TAB_SP;
    toDot(ar::getResult(stmt));
    _cfg_dot += SINGLE_SP;
    _cfg_dot += "=";
    _cfg_dot += SINGLE_SP;
    _cfg_dot += "va_arg(";
    toDot(ar::getPointer(stmt));
    _cfg_dot += ")";
    _cfg_dot += NEWLINE;
  }

  void visit(VA_Copy_ref stmt) {
    _cfg_dot += TAB_SP;
    _cfg_dot += "va_copy(";
    toDot(ar::getDestination(stmt));
    _cfg_dot += ", ";
    toDot(ar::getSource(stmt));
    _cfg_dot += ")";
    _cfg_dot += NEWLINE;
  }

private:
  void toDot(const Operand_ref& o) {
    std::ostringstream buf;
    buf << o;
    _cfg_dot += buf.str();
  }

  void alignToDot(const boost::optional< z_number > alignment) {
    if (alignment) {
      _cfg_dot += ", align ";
      std::ostringstream align_val;
      align_val << *alignment;
      _cfg_dot += align_val.str();
    }
  }

  void toDot(const Internal_Variable_ref& var) { _cfg_dot += ar::getName(var); }

  void toDot(const Var_Addr_Constant_ref& var) { _cfg_dot += ar::getName(var); }

  void toDot(const Type_ref& ty) {
    std::ostringstream buf;
    buf << ty;
    _cfg_dot += buf.str();
  }

  void toDot(const OpRange& args) {
    _cfg_dot += "(";
    for (OpRange::const_iterator I = args.begin(); I != args.end(); ++I) {
      toDot(*I);
      if ((I + 1) != args.end()) {
        _cfg_dot += ", ";
      }
    }
    _cfg_dot += ")";
  }

  void toDot(ArithOp Pred) {
    switch (Pred) {
      case add:
        _cfg_dot += "add";
        return;
      case sub:
        _cfg_dot += "sub";
        return;
      case mul:
        _cfg_dot += "mul";
        return;
      case udiv:
        _cfg_dot += "udiv";
        return;
      case sdiv:
        _cfg_dot += "sdiv";
        return;
      case urem:
        _cfg_dot += "urem";
        return;
      case srem:
        _cfg_dot += "srem";
        return;
      default:
        assert(false && "unreachable");
    }
  }

  void toDot(FPOp Pred) {
    switch (Pred) {
      case fadd:
        _cfg_dot += "fadd";
        return;
      case fsub:
        _cfg_dot += "fsub";
        return;
      case fmul:
        _cfg_dot += "fmul";
        return;
      case fdiv:
        _cfg_dot += "fdiv";
        return;
      case frem:
        _cfg_dot += "frem";
        return;
      default:
        assert(false && "unreachable");
    }
  }

  void toDot(ConvOp op) {
    switch (op) {
      case trunc:
        _cfg_dot += "trunc";
        return;
      case zext:
        _cfg_dot += "zext";
        return;
      case sext:
        _cfg_dot += "sext";
        return;
      case bitcast:
        _cfg_dot += "bitcast";
        return;
      case ptrtoint:
        _cfg_dot += "ptrtoint";
        return;
      case inttoptr:
        _cfg_dot += "inttoptr";
        return;
      case fptrunc:
        _cfg_dot += "fptrunc";
        return;
      case fpext:
        _cfg_dot += "fpext";
        return;
      case fptoui:
        _cfg_dot += "fptoui";
        return;
      case fptosi:
        _cfg_dot += "fptosi";
        return;
      case uitofp:
        _cfg_dot += "uitofp";
        return;
      case sitofp:
        _cfg_dot += "sitofp";
        return;
      default:
        assert(false && "unreachable");
    }
  }

  void toDot(BitwiseOp op) {
    switch (op) {
      case _shl:
        _cfg_dot += "shl";
        return;
      case _lshr:
        _cfg_dot += "lshr";
        return;
      case _ashr:
        _cfg_dot += "ashr";
        return;
      case _and:
        _cfg_dot += "and";
        return;
      case _or:
        _cfg_dot += "or";
        return;
      case _xor:
        _cfg_dot += "xor";
        return;
      default:
        assert(false && "unreachable");
    }
  }

  void toDot(std::string Pred) {
    if (Pred == "ne")
      _cfg_dot += "ne";
    if (Pred == "eq")
      _cfg_dot += "eq";
    if (Pred == "ult")
      _cfg_dot += "ult";
    if (Pred == "ule")
      _cfg_dot += "ule";
    if (Pred == "ugt")
      _cfg_dot += "ugt";
    if (Pred == "uge")
      _cfg_dot += "uge";
    if (Pred == "slt")
      _cfg_dot += "slt";
    if (Pred == "sle")
      _cfg_dot += "sle";
    if (Pred == "sgt")
      _cfg_dot += "sgt";
    if (Pred == "sge")
      _cfg_dot += "sge";

    // Here to translate floating point operators
    if (Pred == "oeq")
      _cfg_dot += "oeq";
    if (Pred == "olt")
      _cfg_dot += "olt";
    if (Pred == "ole")
      _cfg_dot += "ole";
    if (Pred == "one")
      _cfg_dot += "one";
    if (Pred == "ord")
      _cfg_dot += "ord";
    if (Pred == "oge")
      _cfg_dot += "oge";
    if (Pred == "ogt")
      _cfg_dot += "ogt";
    if (Pred == "ueq")
      _cfg_dot += "ueq";
    if (Pred == "une")
      _cfg_dot += "une";
    if (Pred == "uno")
      _cfg_dot += "uno";
    if (Pred == "_ult")
      _cfg_dot += "ult";
    if (Pred == "_ule")
      _cfg_dot += "ule";
  }

  std::string to_str(CompOp Pred) {
    switch (Pred) {
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
        throw error("unsupported predicate");
    }
  }

  std::string to_str(FP_CompOp Pred) {
    switch (Pred) {
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
        return "_ult";
      case _ule:
        return "_ule";
      case une:
        return "une";
      case uno:
        return "uno";
      default:
        throw error("unsupported floating point predicate");
    }
  }

  std::string negatePredicate(CompOp Pred) {
    switch (Pred) {
      case eq:
        return "ne";
      case ne:
        return "eq";
      case ult:
        return "uge";
      case ule:
        return "ugt";
      case slt:
        return "sge";
      case sle:
        return "sgt";
      default:
        throw error("unsupported predicate");
    }
  }

  std::string negatePredicate(FP_CompOp Pred) {
    switch (Pred) {
      case oeq:
        return "one";
      case olt:
        return "oge";
      case ole:
        return "ogt";
      case one:
        return "oeq";
      case ord:
        return "uno";
      case ueq:
        return "une";
      case _ult:
        return "uge";
      case _ule:
        return "ugt";
      case une:
        return "ueq";
      case uno:
        return "ord";
      default:
        throw error("unsupported predicate");
    }
  }

  std::string armour(const std::string& text) {
    unsigned int i;
    std::string out;

    if (text.size() > 0) {
      if (text[0] != '*' && text[0] != '.')
        out += text[0];
      for (i = 1; i < text.size(); i++) {
        switch (text[i]) {
          case '@':
          case '.':
            break;
          default:
            out += text[i];
            break;
        }
      }
    }
    return out;
  }
};

class ARToDotPass : public Pass {
public:
  ARToDotPass() : Pass("cfg-dot", "Translate AR code to dot format.") {}

  virtual ~ARToDotPass() {}

  virtual void execute(Bundle_ref bundle) {
    FuncRange functions = ar::getFunctions(bundle);
    for (FuncRange::iterator it = functions.begin(), et = functions.end();
         it != et;
         ++it) {
      std::string funcName = ar::getName(*it);
      std::string cfg(""); //! to keep the translation
      // prologue
      cfg += "digraph \"CFG for \'" + funcName + "\' function \" {\n";
      cfg += "\tlabel=\"CFG for \'" + funcName + "\' function\";\n";
      // function
      ARToDotVisitor::ARToDotVisitorPtr vis(new ARToDotVisitor());
      ar::accept(*it, vis);
      cfg += vis->cfg_dot();
      // epilogue
      cfg += "}\n";
      // output: dump into a file
      std::string outfile("cfg." + funcName + ".dot");
      std::ofstream fout;
      fout.open(outfile.c_str());
      fout << cfg;
      fout.close();
    }
  }
};

} // end namespace passes
} // end namespace arbos

extern "C" arbos::Pass* init() {
  return new arbos::passes::ARToDotPass();
}
