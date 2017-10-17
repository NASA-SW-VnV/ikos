/*******************************************************************************
 *
 * An LLVM opt plugin that generates the AR representation in s-expression
 * used by ARBOS.
 *
 * Authors: Nija Shi
 *          Arnaud Venet
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
#include <map>
#include <memory>
#include <sstream>
#include <vector>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/optional.hpp>

#include "llvm/ADT/APFloat.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GetElementPtrTypeIterator.h"
#include "llvm/IR/GlobalAlias.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"

#include <arbos/common/bignums.hpp>
#include <arbos/common/types.hpp>
#include <arbos/io/s_expressions.hpp>
#include <arbos/semantics/fp.hpp>

using namespace llvm;
using namespace arbos;

static cl::opt< bool > DOT_OUTPUT(
    "dot-ar", cl::desc("Output DOT file for each function"));

static cl::opt< bool > ENABLE_GLOBAL_INIT(
    "enable-gvinit",
    cl::desc("Enable lowering transformation of zero initializers"));

class arbos_pass_error : public arbos_error {
public:
  arbos_pass_error(std::string msg) : arbos_error("[ARBOS Pass] " + msg) {}
}; // class arbos_pass_error

namespace {

class UIDGenerator {
private:
  static uint64_t currentUID;

public:
  inline static uint64_t nextUID() { return currentUID++; }
  inline static uint64_t getCurrentUID() { return currentUID; }
}; // class UIDGenerator

class ARType;
class ARConstant;
class ARGlobalVar;
class ARCode;

class GlobalReferences {
private:
  Module& _module;
  LLVMContext* _llvm_context;
  std::unordered_map< std::string, uint64_t > _files;
  std::unordered_map< const Type*, std::shared_ptr< ARType > > _data_types;
  std::unordered_map< GlobalVariable*, std::shared_ptr< ARGlobalVar > >
      _global_vars; // var name to initializer
  uint64_t _idnum;
  std::unordered_map< Value*, uint64_t > _valmap;
  s_expression_ref _current_debug_info;

private:
  void print_files(std::ostream& outf);
  void print_data_types(std::ostream& outf);
  void print_global_vars(std::ostream&);
  void translate_initializer(ARCode&, s_expression, Constant*, z_number);
  void translate_zero_initializer(ARCode&, s_expression, const Type*, z_number);

public:
  GlobalReferences(Module& m)
      : _module(m), _llvm_context(&m.getContext()), _idnum(1) {}
  const DataLayout& getModuleDataLayout() { return _module.getDataLayout(); }
  uint64_t get_type_refid(const Type*);
  uint64_t get_file_id(const std::string);
  std::string get_new_variable();
  s_expression get_value(ARCode&, Value*);
  s_expression translate_type(const Type*);
  s_expression generate_int_constant(z_number, const Type*);
  s_expression generate_zero_constant(const Type*);
  void translate_index(ARCode&, const Type*, Value*, s_expression, std::string);
  s_expression translate_getelementptr(ARCode&, Instruction*);
  s_expression translate_extractvalue(ARCode&, ExtractValueInst*);
  s_expression translate_insertvalue(ARCode&, InsertValueInst*);
  s_expression translate_extractelement(ARCode&, ExtractElementInst*);
  s_expression translate_insertelement(ARCode&, InsertElementInst*);
  s_expression translate_constant(ARCode&, Constant*);
  s_expression translate_constant_local_addr(ARCode&, Value*);
  void translate_initializer(ARCode&, s_expression, Constant*);
  void generate_abstract_memory(ARCode&, s_expression, const PointerType*);
  void identify_global_variables(Module&);
  void set_debug_info(GlobalVariable*);
  void set_debug_info(Instruction*);
  void reset_debug_info();
  s_expression get_debug_sexpr();
  s_expression translate_srcloc(const int, const int, const std::string);
  void printAR(std::ostream&);
  void clear();
};

std::shared_ptr< GlobalReferences > refs;

s_expression create_name_sexpr(std::string name) {
  s_expression_ostream n("name");
  n << string_atom(name);
  return n.expr();
}

s_expression create_ty_ref_sexpr(uint64_t ref) {
  s_expression_ostream ty("ty");
  ty << index64_atom(ref);
  return ty.expr();
}

s_expression create_align_sexpr(unsigned int alignment) {
  s_expression_ostream align("align");
  align << z_number_atom(alignment);
  return align.expr();
}

s_expression create_var_sexpr(std::string name, uint64_t ty_id) {
  s_expression_ostream v("var");
  v << create_name_sexpr(name);
  v << create_ty_ref_sexpr(ty_id);
  return v.expr();
}

s_expression create_var_sexpr(std::string name,
                              uint64_t ty_id,
                              unsigned int alignment) {
  s_expression_ostream v("var");
  v << create_name_sexpr(name);
  v << create_ty_ref_sexpr(ty_id);
  v << create_align_sexpr(alignment);
  return v.expr();
}

s_expression gen_sexpr_assign(s_expression lhs, s_expression rhs) {
  s_expression_ostream s("assign");
  s_expression_ostream l("lhs"), r("rhs");
  s << (l << lhs).expr() << (r << rhs).expr() << refs->get_debug_sexpr();
  return s.expr();
}

s_expression gen_sexpr_call(s_expression ret,
                            s_expression called_value,
                            std::vector< s_expression_ref > arg_operands,
                            bool is_inline_asm) {
  // Generic case: direct call to a function that is not intrinsic or indirect
  // call

  s_expression_ostream k("call");
  // Return value/type
  s_expression_ostream s_result("ret");
  s_result << ret;
  k << s_result.expr();

  // Destination/type
  s_expression_ostream s_dest("callee");
  s_dest << called_value;
  k << s_dest.expr();

  // Arguments/types
  s_expression_ostream s_args("args");
  std::vector< s_expression_ref >::iterator i = arg_operands.begin();
  for (; i != arg_operands.end(); i++) {
    s_expression_ostream s_arg("a");
    s_arg << **i;
    s_args << s_arg.expr();
  }

  k << s_args.expr();

  // Attribute that indicates if this call is an inline asm statement
  s_expression_ostream s_inline_asm("inlineAsm");
  s_inline_asm << string_atom(is_inline_asm ? "true" : "false");
  k << s_inline_asm.expr();

  k << refs->get_debug_sexpr();
  return k.expr();
}

void append_type_sizes(s_expression_ostream& o,
                       z_number real_size,
                       z_number store_size) {
  o << (s_expression_ostream("realsize") << z_number_atom(real_size)).expr()
    << (s_expression_ostream("storesize") << z_number_atom(store_size)).expr();
}

bool has_zero_initializer(GlobalVariable* gv) {
  if (!gv->hasInitializer())
    return false;

  Constant* init = gv->getInitializer();
  return ConstantAggregateZero::classof(init);
}

/**
 * Output DOT files
 */
template < class Name >
class vertex_writer {
private:
  Name _name;
  std::unordered_map< std::string, std::vector< std::string > >& _bblocks;

public:
  vertex_writer(
      Name name,
      std::unordered_map< std::string, std::vector< std::string > >& bblocks)
      : _name(name), _bblocks(bblocks) {}

  template < typename VertexOrEdge >
  void operator()(std::ostream& out, const VertexOrEdge& v) const {
    std::string bb(_name[v]);
    out << "[label=\"" << bb << ":";
    std::vector< std::string >::iterator inst = _bblocks[bb].begin();
    for (; inst != _bblocks[bb].end(); inst++) {
      out << "\\l" << *inst;
    }
    out << "\\l\" shape=box]";
  }
};

template < typename Name >
inline vertex_writer< Name > make_vertex_writer(
    Name n,
    std::unordered_map< std::string, std::vector< std::string > >& bblocks) {
  return vertex_writer< Name >(n, bblocks);
}

class ARCode {
private:
  std::string _entry_block;
  std::string _exit_block;
  std::string _unreachable_block;
  std::string _resume_block;
  std::unordered_map< std::string,
                      std::vector< boost::optional< s_expression > > >
      _bblocks; // bblock name ID to a vertex; a vertex consists of a list of
  // stmts
  std::string
      _current_ar_bblock; // the current bblock in the ar cfg to write to
  std::unordered_map< std::string, std::vector< std::string > >
      _transitions; // bblock -> other bblocks

  bool is_part_of_cycle(std::string bblock) {
    std::vector< std::string > nodes = _transitions[bblock];
    return find(nodes.begin(), nodes.end(), bblock) != nodes.end();
  }

  /*! A basic block may be split into multiple smaller basic blocks due to
   *        transformation of i/fcmp (boolena ops) and select instructions.
   *        This map maintains a counter for each split per basic block and
   * opcode.
   */
  std::unordered_map< std::string, int > _splitct_map;

  //! Associated to splitct_map, this method returns the next counter.
  inline std::string get_next_split_ct(std::string bblock,
                                       std::string opcodename) {
    std::string key = bblock + "+" + opcodename;
    if (_splitct_map.find(key) == _splitct_map.end()) {
      _splitct_map[key] = 1;
      return "";
    } else {
      std::string cur_splitct = std::to_string(_splitct_map[key]) + "_";
      _splitct_map[key]++;
      return cur_splitct;
    }
  }

public:
  ARCode()
      : _entry_block(""),
        _exit_block(""),
        _unreachable_block(""),
        _resume_block("") {}
  void set_entry_block(std::string block) { _entry_block = block; }

  void set_exit_block(std::string block) { _exit_block = block; }
  void set_unreachable_block(std::string block) { _unreachable_block = block; }
  void set_ehresume_block(std::string block) { _resume_block = block; }

  inline std::string get_exit_block() { return _exit_block; }

  void enter_basic_block(std::string block);
  std::string get_current_block();
  void add_statement(s_expression_ref c);
  void add_transition(std::string src, std::string dest);
  void remove_transition(std::string src, std::string dest);
  void mark_as_referenced_bblock(std::string bblock_name);
  std::string get_new_destination(std::string orig_src, std::string orig_dest);
  std::string get_new_source(std::string orig_src, std::string orig_dest);
  void route_to_new_dest(std::string orig_src, std::string orig_dest);
  void split_blocks(Instruction*, Value* assertion);
  s_expression expr();
  void printDOT(std::ostream& out);
};

class ARFunction {
private:
  std::string _name;
  uint64_t _func_type_id;
  std::vector< boost::optional< s_expression > > _args;
  std::vector< boost::optional< s_expression > > _local_vars;
  Instruction* _next_llvm_inst;
  ARCode _cfg;

private:
  s_expression_ref translate_instruction(Instruction*);
  s_expression_ref gen_sexpr_intrinsic_call(ARCode&, CallInst*);

public:
  ARFunction(Function*,
             BasicBlock* return_block,
             BasicBlock* unreachable_block);
  void printAR(std::ostream& outf);
  void printDOT();
};

class ARGlobalVar {
private:
  uint64_t _id;
  boost::optional< s_expression > _s_expr_ref;

public:
  ARGlobalVar(uint64_t id) : _id(id) {}
  ARGlobalVar(uint64_t id, std::string def) : _id(id) {}
  uint64_t get_id() { return _id; }
  s_expression get_definition() { return _s_expr_ref.get(); }
  inline void set_definition(s_expression s) { _s_expr_ref = s; }
};

class ARConstant {
private:
  uint64_t _id;
  boost::optional< s_expression > _s_expr_ref;

public:
  ARConstant(uint64_t id) : _id(id) {}
  ARConstant(uint64_t id, std::string def) : _id(id) {}
  uint64_t get_id() { return _id; }
  s_expression get_definition() { return _s_expr_ref.get(); }
  inline void set_definition(s_expression s) { _s_expr_ref = s; }
};

class ARType {
private:
  uint64_t _id;
  boost::optional< s_expression > _s_expr_ref;

private:
  std::string translate(const Type*);

public:
  ARType(uint64_t id) : _id(id) {}
  ARType(uint64_t id, std::string desc) : _id(id) {}
  uint64_t get_id() { return _id; }
  s_expression get_description() { return _s_expr_ref.get(); }
  inline void set_description(s_expression s) { _s_expr_ref = s; }
};

class ARPass : public ModulePass {
private:
  bool _is_little_endian;
  unsigned int _ptr_size;
  std::string _target_triple;
  std::unordered_map< std::string, boost::optional< s_expression > >
      _meta_info; // key-value pairs
  std::vector< std::shared_ptr< ARFunction > > _functions;

public:
  static char ID;

public:
  ARPass() : ModulePass(ID) {}
  virtual bool runOnModule(Module&);
  virtual void getAnalysisUsage(AnalysisUsage& au) const {
    au.setPreservesAll();
    au.addRequired< UnifyFunctionExitNodes >();
  }

private:
  void populate_metainfo();

  void printAR();
  void printDOT();
  void print_meta_info(std::ostream&);
  void print_functions(std::ostream&);
}; // class ARPass

} // local namespace

uint64_t UIDGenerator::currentUID = 1;
char ARPass::ID = 0;
RegisterPass< ARPass > X("arbos",
                         "Generation of the abstract semantics for ARBOS",
                         false,
                         false);

/**
 * ARPass implementation
 */
bool ARPass::runOnModule(Module& m) {
  try {
    _is_little_endian = m.getDataLayout().isLittleEndian();
    _ptr_size = m.getDataLayout().getPointerSize();
    _target_triple = m.getTargetTriple();
    refs = std::shared_ptr< GlobalReferences >(new GlobalReferences(m));

    // 1. Collect meta info
    populate_metainfo();

    // 2. Process global variables
    refs->identify_global_variables(m);

    // 3. Process functions
    for (Module::iterator f = m.begin(); f != m.end(); f++) {
      if (f->isDeclaration())
        continue;

      UnifyFunctionExitNodes& ufen = getAnalysis< UnifyFunctionExitNodes >(*f);

      _functions.push_back(std::shared_ptr< ARFunction >(
          new ARFunction(&*f,
                         ufen.getReturnBlock(),
                         ufen.getUnreachableBlock())));
    }

    printAR(); // dump to AR

    if (DOT_OUTPUT)
      printDOT();

    return false; // returns false since no modification to
    // the LLVM IR should be done in this pass.
  } catch (arbos_pass_error& e) {
    errs() << "*** Fatal error: " << e.message() << "\n";
    // Clean up global references
    _meta_info.clear();
    refs->clear();
    exit(1);
  }
}

void ARPass::populate_metainfo() {
  s_expression_ostream sizeofptr_ss("sizeofptr");
  sizeofptr_ss << z_number_atom(_ptr_size);
  _meta_info["sizeofptr"] = sizeofptr_ss.expr();

  s_expression_ostream endian_ss("endianess");
  std::ostringstream endian_str;
  endian_str << (_is_little_endian ? "Little" : "Big") << "Endian";
  endian_ss << string_atom(endian_str.str());
  _meta_info["endianess"] = endian_ss.expr();

  s_expression_ostream targettriple_ss("targettriple");
  targettriple_ss << string_atom(_target_triple);
  _meta_info["targettriple"] = targettriple_ss.expr();
}

void ARPass::printAR() {
  print_meta_info(std::cout);
  refs->printAR(std::cout);
  print_functions(std::cout);
}

void ARPass::printDOT() {
  std::vector< std::shared_ptr< ARFunction > >::iterator f = _functions.begin();
  for (; f != _functions.end(); f++) {
    (*f)->printDOT();
  }
}

void ARPass::print_meta_info(std::ostream& outf) {
  std::unordered_map< std::string, boost::optional< s_expression > >::iterator
      p = _meta_info.begin();
  for (; p != _meta_info.end(); p++) {
    s_expression_ostream s("metainfo");
    s << p->second.get();
    outf << s.expr() << std::endl;
  }
  _meta_info.clear();
}

void ARPass::print_functions(std::ostream& outf) {
  std::vector< std::shared_ptr< ARFunction > >::iterator p = _functions.begin();
  for (; p != _functions.end(); p++) {
    (*p)->printAR(outf);
  }
}

/**
 * GlobalReferences implementation
 */

void GlobalReferences::identify_global_variables(Module& m) {
  for (Module::global_iterator global = m.global_begin();
       global != m.global_end();
       global++) {
    _global_vars[&*global] = std::shared_ptr< ARGlobalVar >(
        new ARGlobalVar(UIDGenerator::nextUID()));
  }

  for (std::unordered_map< GlobalVariable*,
                           std::shared_ptr< ARGlobalVar > >::iterator p =
           _global_vars.begin();
       p != _global_vars.end();
       p++) {
    GlobalVariable* gv = p->first;
    refs->set_debug_info(gv);

    std::string name = gv->getName().str();
    s_expression_ostream s("gv");
    s_expression_ostream s_name("name");
    s_name << string_atom(name);
    s << s_name.expr();

    s_expression_ostream ty("ty");
    ty << index64_atom(get_type_refid(gv->getType()));
    s << ty.expr();

    s << create_align_sexpr(gv->getAlignment());

    if (!ENABLE_GLOBAL_INIT && has_zero_initializer(gv)) {
      // Global variables without explicit initializers at the source level
      // are translated as symbols with common linkage (or private unnamed_addr
      // in LLVM 3.0 or above) and are being added with zero initializers. Due
      // to
      // performance, by default the AR pass avoids lowering zero initializers
      // if there have not been explicitly defined in the source code. For
      // example,
      // lowering a zero initializer for a huge array can cause significant
      // performance issues. In this case, we insert an empty code body.
      s_expression_ostream code("code");
      s << code.expr();
    } else {
      // Create initializer
      s_expression_ostream gv_cst("cst");
      s_expression_ostream k("globalvariableref");
      s_expression_ostream gv_name("name");
      gv_name << string_atom(gv->getName().str());
      k << gv_name.expr();
      s_expression_ostream gv_s("gv");
      gv_s << index64_atom(_global_vars[gv]->get_id());
      k << gv_s.expr();
      k << create_ty_ref_sexpr(get_type_refid(gv->getType()));
      gv_cst << k.expr();

      ARCode init_code;
      init_code.enter_basic_block("init");
      init_code.set_entry_block("init");

      if (gv->hasInitializer()) {
        Constant* init = gv->getInitializer();
        translate_initializer(init_code, gv_cst.expr(), init);
      } else {
        generate_abstract_memory(init_code, gv_cst.expr(), gv->getType());
      }

      init_code.set_exit_block(init_code.get_current_block());
      s << init_code.expr();
    }

    s << get_debug_sexpr();
    refs->reset_debug_info();
    _global_vars[gv]->set_definition(s.expr());
  }
}

uint64_t GlobalReferences::get_file_id(const std::string file) {
  if (_files.find(file) == _files.end()) {
    _files[file] = UIDGenerator::nextUID();
  }
  return _files[file];
}

std::string GlobalReferences::get_new_variable() {
  uint64_t id = _idnum++;
  std::ostringstream ss;
  ss << "__v:" << id;
  return ss.str();
}

s_expression GlobalReferences::get_value(ARCode& cfg, Value* v) {
  // Constant
  if (Constant* cst = dyn_cast< Constant >(v)) {
    return translate_constant(cfg, cst);
  }

  if (AllocaInst* alloca = dyn_cast< AllocaInst >(v)) {
    return translate_constant_local_addr(cfg, alloca);
  }

  // Named value
  if (v->hasName()) {
    std::string realname = v->getName();
    if (realname != "") {
      return create_var_sexpr(realname, get_type_refid(v->getType()));
    }
  }

  // Anonymous value
  std::unordered_map< Value*, uint64_t >::iterator p = _valmap.find(v);
  uint64_t id;

  if (p == _valmap.end()) {
    id = _idnum;
    _valmap[v] = _idnum;
    _idnum++;
  } else {
    id = p->second;
  }

  std::ostringstream ss;
  ss << "__v:" << id;
  return create_var_sexpr(ss.str(), get_type_refid(v->getType()));
}

uint64_t GlobalReferences::get_type_refid(const Type* t) {
  if (_data_types.find(t) == _data_types.end()) {
    _data_types[t] =
        std::shared_ptr< ARType >(new ARType(UIDGenerator::nextUID()));
    _data_types[t]->set_description(translate_type(t));
  }

  return _data_types[t]->get_id();
}

DIVariable* find_debug_global_declare(GlobalVariable* gv) {
  SmallVector< DIGlobalVariableExpression*, 4 > di_gvs;
  gv->getDebugInfo(di_gvs);

  for (DIGlobalVariableExpression* di_gv : di_gvs) {
    if (di_gv->getVariable()) {
      return di_gv->getVariable();
    }
  }

  return nullptr;
}

void GlobalReferences::set_debug_info(Instruction* inst) {
  std::string location;
  int line, column;

  DILocation* di_loc = inst->getDebugLoc().get();

  if (di_loc && di_loc->isResolved()) {
    line = di_loc->getLine();
    column = di_loc->getColumn();
    StringRef File = di_loc->getFile()->getFilename();
    StringRef Dir = di_loc->getDirectory();
    if (File.startswith("/")) {
      location += File;
    } else {
      location += Dir;
      if (!Dir.endswith("/")) {
        location += "/";
      }
      location += File;
    }
    s_expression_ostream debug("debug");
    debug << translate_srcloc(line, column, location);
    _current_debug_info = debug.expr();
  } else {
    s_expression_ostream debug("debug");
    debug << translate_srcloc(-1, -1, "?");
    _current_debug_info = debug.expr();
  }
}

Instruction* find_dbg_user(GlobalVariable* gv) {
  // Iterate over each use.
  for (Value::use_iterator u = gv->use_begin(); u != gv->use_end(); u++) {
    User* user = u->getUser();

    if (isa< Instruction >(user)) {
      Instruction* inst = dyn_cast< Instruction >(user);
      if (inst->getDebugLoc()) {
        return inst;
      }
    }
  }

  return nullptr;
}

void GlobalReferences::set_debug_info(GlobalVariable* gv) {
  std::string location;
  int line;

  DIVariable* di_gv = find_debug_global_declare(gv);
  if (di_gv) {
    line = di_gv->getLine();
    StringRef File = di_gv->getFile()->getFilename();
    StringRef Dir = di_gv->getFile()->getDirectory();
    if (File.startswith("/")) {
      location += File;
    } else {
      location += Dir;
      if (!Dir.endswith("/")) {
        location += "/";
      }
      location += File;
    }
    s_expression_ostream debug("debug");
    debug << translate_srcloc(line, 1, location);
    _current_debug_info = debug.expr();
    return;
  } else {
    // Such global variables do not come with explicit dbg
    // information (typically gv with common, private, and
    // internal linkages). Here we use the dbg info of its
    // first use.
    Instruction* dbg_inst = find_dbg_user(gv);
    if (dbg_inst) {
      set_debug_info(dbg_inst);
      return;
    }
  }

  // Cannot determine location for gv
  s_expression_ostream debug("debug");
  debug << translate_srcloc(-1, -1, "?");
  _current_debug_info = debug.expr();
}

void GlobalReferences::reset_debug_info() {
  _current_debug_info = s_expression_ref();
}

s_expression GlobalReferences::get_debug_sexpr() {
  if (_current_debug_info) {
    return *_current_debug_info;
  } else {
    throw arbos_pass_error(
        "Internal error: cannot find source location information");
  }
}

s_expression GlobalReferences::translate_srcloc(const int line,
                                                const int column,
                                                const std::string location) {
  s_expression_ostream s("srcloc");
  uint64_t file_ref = get_file_id(location);

  s_expression_ostream s_line("line");
  s_line << z_number_atom(line);
  s << s_line.expr();

  s_expression_ostream s_col("col");
  s_col << z_number_atom(column);
  s << s_col.expr();

  s_expression_ostream s_file("file");
  s_file << index64_atom(file_ref);
  s << s_file.expr();

  return s.expr();
}

void GlobalReferences::translate_zero_initializer(ARCode& cfg,
                                                  s_expression global_var,
                                                  const Type* ty,
                                                  z_number offset) {
  const DataLayout& d = refs->getModuleDataLayout();
  if (StructType* tstruct =
          const_cast< StructType* >(dyn_cast< StructType >(ty))) {
    for (unsigned int i = 0; i < tstruct->getNumContainedTypes(); ++i) {
      z_number elem_offset = d.getStructLayout(tstruct)->getElementOffset(i);
      translate_zero_initializer(cfg,
                                 global_var,
                                 tstruct->getElementType(i),
                                 offset + elem_offset);
    }
  } else if (const ArrayType* tarray = dyn_cast< ArrayType >(ty)) {
    Type* telement = tarray->getElementType();
    z_number elem_size = d.getTypeAllocSize(telement);
    for (unsigned int idx = 0; idx < tarray->getNumElements(); ++idx) {
      z_number element_offset = z_number::from_ulong(idx) * elem_size;
      translate_zero_initializer(cfg,
                                 global_var,
                                 telement,
                                 offset + element_offset);
    }
  } else if (const VectorType* tvec = dyn_cast< VectorType >(ty)) {
    Type* telement = tvec->getElementType();
    z_number elem_size = d.getTypeAllocSize(telement);
    for (unsigned int idx = 0; idx < tvec->getNumElements(); ++idx) {
      z_number element_offset = z_number::from_ulong(idx) * elem_size;
      translate_zero_initializer(cfg,
                                 global_var,
                                 telement,
                                 offset + element_offset);
    }
  } else {
    const Type* offset_type = IntegerType::get(*_llvm_context, 64);
    PointerType* ptr_ty = PointerType::getUnqual(const_cast< Type* >(ty));
    s_expression offset_var =
        create_var_sexpr(get_new_variable(), get_type_refid(ptr_ty));
    s_expression_ostream shift("ptrshift");
    s_expression_ostream shift_lhs("lhs");
    shift_lhs << offset_var;
    s_expression_ostream shift_rhs("rhs");
    shift_rhs << global_var << generate_int_constant(offset, offset_type);
    shift << shift_lhs.expr() << shift_rhs.expr() << get_debug_sexpr();
    cfg.add_statement(shift.expr());
    s_expression_ostream store("store");
    s_expression_ostream store_lhs("lhs");
    store_lhs << offset_var;
    s_expression_ostream store_rhs("rhs");
    store_rhs << generate_zero_constant(ty);
    s_expression_ostream align("align");
    store << store_lhs.expr() << store_rhs.expr() << align.expr()
          << get_debug_sexpr();
    cfg.add_statement(store.expr());
  }
}

void GlobalReferences::translate_initializer(ARCode& cfg,
                                             s_expression global_var,
                                             Constant* cst,
                                             z_number offset) {
  const DataLayout& d = refs->getModuleDataLayout();
  if (dyn_cast< ConstantAggregateZero >(cst)) {
    translate_zero_initializer(cfg, global_var, cst->getType(), offset);
  } else if (ConstantDataArray* array = dyn_cast< ConstantDataArray >(cst)) {
    const ArrayType* tarray = dyn_cast< ArrayType >(cst->getType());
    Type* telement = const_cast< Type* >(tarray->getElementType());
    z_number elem_size = d.getTypeAllocSize(telement);
    for (unsigned int idx = 0; idx < array->getNumElements(); idx++) {
      Constant* elem_cst = array->getElementAsConstant(idx);
      z_number elem_offset = offset + (z_number::from_ulong(idx) * elem_size);
      translate_initializer(cfg, global_var, elem_cst, elem_offset);
    }
  } else if (dyn_cast< ConstantArray >(cst)) {
    const ArrayType* tarray = dyn_cast< ArrayType >(cst->getType());
    Type* telement = const_cast< Type* >(tarray->getElementType());
    z_number elem_size = d.getTypeAllocSize(telement);
    Constant::op_iterator p = cst->op_begin(), e = cst->op_end();
    for (unsigned int idx = 0; p != e; ++idx, ++p) {
      Constant* elem_cst = dyn_cast< Constant >(*p);
      z_number elem_offset = offset + (z_number::from_ulong(idx) * elem_size);
      translate_initializer(cfg, global_var, elem_cst, elem_offset);
    }
  } else if (dyn_cast< ConstantVector >(cst)) {
    const VectorType* tvec = dyn_cast< VectorType >(cst->getType());
    Type* telement = const_cast< Type* >(tvec->getElementType());
    z_number elem_size = d.getTypeAllocSize(telement);
    Constant::op_iterator p = cst->op_begin(), e = cst->op_end();
    for (unsigned int idx = 0; p != e; ++idx, ++p) {
      Constant* elem_cst = dyn_cast< Constant >(*p);
      z_number elem_offset = offset + (z_number::from_ulong(idx) * elem_size);
      translate_initializer(cfg, global_var, elem_cst, elem_offset);
    }
  } else if (dyn_cast< ConstantStruct >(cst)) {
    StructType* tstruct = dyn_cast< StructType >(cst->getType());
    Constant::op_iterator p = cst->op_begin(), e = cst->op_end();
    for (uint64_t i = 0; p != e; ++i, ++p) {
      Constant* elem_cst = dyn_cast< Constant >(*p);
      z_number elem_offset =
          offset + d.getStructLayout(tstruct)->getElementOffset(i);
      translate_initializer(cfg, global_var, elem_cst, elem_offset);
    }
  } else if (GlobalAlias* alias = dyn_cast< GlobalAlias >(cst)) {
    translate_initializer(cfg, global_var, alias->getAliasee(), offset);
  } else {
    const Type* offset_type = IntegerType::get(*_llvm_context, 64);
    Type* cst_ty = cst->getType();
    PointerType* ptr_ty = PointerType::getUnqual(cst_ty);
    s_expression offset_var =
        create_var_sexpr(get_new_variable(), get_type_refid(ptr_ty));
    s_expression_ostream shift("ptrshift");
    s_expression_ostream shift_lhs("lhs");
    shift_lhs << offset_var;
    s_expression_ostream shift_rhs("rhs");
    shift_rhs << global_var << generate_int_constant(offset, offset_type);
    shift << shift_lhs.expr() << shift_rhs.expr() << get_debug_sexpr();
    cfg.add_statement(shift.expr());
    s_expression_ostream store("store");
    s_expression_ostream store_lhs("lhs");
    store_lhs << offset_var;
    s_expression_ostream store_rhs("rhs");
    store_rhs << translate_constant(cfg, cst);
    s_expression_ostream align("align");
    store << store_lhs.expr() << store_rhs.expr() << align.expr()
          << get_debug_sexpr();
    cfg.add_statement(store.expr());
  }
}

void GlobalReferences::translate_initializer(ARCode& cfg,
                                             s_expression global_var,
                                             Constant* cst) {
  translate_initializer(cfg, global_var, cst, 0);
}

void GlobalReferences::generate_abstract_memory(ARCode& cfg,
                                                s_expression global_var,
                                                const PointerType* ty) {
  const DataLayout& d = refs->getModuleDataLayout();
  s_expression_ostream am("abstractmemory"), ptr("ptr"), len("len");
  const Type* len_ty = IntegerType::get(*_llvm_context, 64);
  am << (ptr << global_var).expr()
     << (len << generate_int_constant(d.getTypeAllocSize(ty->getElementType()),
                                      len_ty))
            .expr()
     << get_debug_sexpr();
  cfg.add_statement(am.expr());
}

s_expression GlobalReferences::generate_int_constant(z_number n,
                                                     const Type* ty) {
  s_expression_ostream cst("cst");
  s_expression_ostream k("constantint");
  s_expression_ostream value_s("val");
  value_s << z_number_atom(n);
  k << value_s.expr();
  k << create_ty_ref_sexpr(get_type_refid(ty));
  cst << k.expr();
  return cst.expr();
}

s_expression GlobalReferences::generate_zero_constant(const Type* ty) {
  const DataLayout& d = refs->getModuleDataLayout();

  s_expression_ostream zero("cst");
  if (dyn_cast< IntegerType >(ty)) {
    return generate_int_constant(0, ty);
  } else if (dyn_cast< FunctionType >(ty)) {
    s_expression_ostream k("constantpointernull");
    k << create_ty_ref_sexpr(get_type_refid(ty));
    zero << k.expr();
  } else if (StructType* tstruct =
                 const_cast< StructType* >(dyn_cast< StructType >(ty))) {
    s_expression_ostream k("range");
    s_expression_ostream values("values");
    for (unsigned int i = 0; i < tstruct->getNumContainedTypes(); ++i) {
      s_expression_ostream v_s("value");
      z_number offset = d.getStructLayout(tstruct)->getElementOffset(i);
      v_s << z_number_atom(offset)
          << generate_zero_constant(tstruct->getElementType(i));
      values << v_s.expr();
    }
    k << values.expr();
    k << create_ty_ref_sexpr(get_type_refid(ty));
    zero << k.expr();
  } else if (const ArrayType* tarray = dyn_cast< ArrayType >(ty)) {
    Type* telement = const_cast< Type* >(tarray->getElementType());
    z_number elem_size = d.getTypeAllocSize(telement);
    s_expression_ostream k("range");
    s_expression_ostream values("values");
    for (unsigned int idx = 0; idx < tarray->getNumElements(); ++idx) {
      s_expression_ostream v_s("value");
      v_s << z_number_atom(z_number::from_ulong(idx) * elem_size)
          << generate_zero_constant(telement);
      values << v_s.expr();
    }
    k << values.expr();
    k << create_ty_ref_sexpr(get_type_refid(ty));
    zero << k.expr();
  } else if (const VectorType* tvec = dyn_cast< VectorType >(ty)) {
    Type* telement = const_cast< Type* >(tvec->getElementType());
    z_number elem_size = d.getTypeAllocSize(telement);
    s_expression_ostream k("range");
    s_expression_ostream values("values");
    for (unsigned int idx = 0; idx < tvec->getNumElements(); ++idx) {
      s_expression_ostream v_s("value");
      v_s << z_number_atom(z_number::from_ulong(idx) * elem_size)
          << generate_zero_constant(telement);
      values << v_s.expr();
    }
    k << values.expr();
    k << create_ty_ref_sexpr(get_type_refid(ty));
    zero << k.expr();
  } else if (dyn_cast< PointerType >(ty)) {
    s_expression_ostream k("constantpointernull");
    k << create_ty_ref_sexpr(get_type_refid(ty));
    zero << k.expr();
  } else if (ty->isFloatingPointTy()) {
    const fltSemantics* fsem;
    if (ty->getPrimitiveSizeInBits() == 16) {
      fsem = &APFloat::IEEEhalf();
    } else if (ty->isFloatTy()) {
      fsem = &APFloat::IEEEsingle();
    } else if (ty->isDoubleTy()) {
      fsem = &APFloat::IEEEdouble();
    } else if (ty->isX86_FP80Ty()) {
      fsem = &APFloat::x87DoubleExtended();
    } else if (ty->isFP128Ty()) {
      fsem = &APFloat::IEEEquad();
    } else if (ty->isPPC_FP128Ty()) {
      fsem = &APFloat::PPCDoubleDouble();
    } else {
      std::ostringstream buf;
      buf << "Unknown floating-point type";
      throw arbos_pass_error(buf.str());
    }
    s_expression_ostream k("constantfp");
    APFloat z(*fsem, "0.0");
    char buf[256];
    z.convertToHexString(buf, 0, false, APFloat::rmNearestTiesToEven);
    s_expression_ostream v("val");
    v << fp_number_atom(fp_number(buf));
    k << v.expr();
    k << create_ty_ref_sexpr(get_type_refid(ty));
    zero << k.expr();
  } else if (ty->isLabelTy()) {
    throw arbos_pass_error("Labels as values are not supported");
    //  } else if (dyn_cast< OpaqueType >(ty)) {
    //    throw arbos_pass_error("Opaque type found in initializer");
  } else {
    throw arbos_pass_error("Unknown type found");
  }
  return zero.expr();
}

void GlobalReferences::translate_index(ARCode& cfg,
                                       const Type* ty,
                                       Value* idx,
                                       s_expression base_offset,
                                       std::string shift_op) {
  const DataLayout& d = refs->getModuleDataLayout();

  if (StructType* tstruct =
          const_cast< StructType* >(dyn_cast< StructType >(ty))) {
    if (ConstantInt* field = dyn_cast< ConstantInt >(idx)) {
      z_number offset_i =
          d.getStructLayout(tstruct)->getElementOffset(field->getZExtValue());
      s_expression offset = generate_int_constant(offset_i, field->getType());
      s_expression_ostream shift((string_atom(shift_op)));
      s_expression_ostream s_lhs("lhs");
      s_lhs << base_offset;
      s_expression_ostream s_rhs("rhs");
      s_rhs << base_offset << offset;
      shift << s_lhs.expr() << s_rhs.expr() << get_debug_sexpr();
      cfg.add_statement(shift.expr());
    } else {
      throw arbos_pass_error(
          "Malformed GetElementPtr/ExtractValue/InsertValue (non-constant "
          "struct field index)");
    }
  } else if (const ArrayType* tarray = dyn_cast< ArrayType >(ty)) {
    s_expression index = get_value(cfg, idx);
    const Type* idx_type = idx->getType();
    s_expression offset_idx =
        create_var_sexpr(get_new_variable(), get_type_refid(idx_type));
    s_expression_ostream m_lhs("lhs");
    m_lhs << offset_idx;
    s_expression element_size =
        generate_int_constant(d.getTypeAllocSize(tarray->getElementType()),
                              idx_type);
    s_expression_ostream m_rhs("rhs");
    m_rhs << element_size << index;
    s_expression_ostream mul_idx("mul");
    mul_idx << m_lhs.expr() << m_rhs.expr() << get_debug_sexpr();
    s_expression_ostream shift((string_atom(shift_op)));
    s_expression_ostream s_lhs("lhs");
    s_lhs << base_offset;
    s_expression_ostream s_rhs("rhs");
    s_rhs << base_offset << offset_idx;
    shift << s_lhs.expr() << s_rhs.expr() << get_debug_sexpr();
    cfg.add_statement(mul_idx.expr());
    cfg.add_statement(shift.expr());
  } else if (const VectorType* tvec = dyn_cast< VectorType >(ty)) {
    s_expression index = get_value(cfg, idx);
    const Type* idx_type = idx->getType();
    s_expression offset_idx =
        create_var_sexpr(get_new_variable(), get_type_refid(idx_type));
    s_expression_ostream m_lhs("lhs");
    m_lhs << offset_idx;
    s_expression element_size =
        generate_int_constant(d.getTypeAllocSize(tvec->getElementType()),
                              idx_type);
    s_expression_ostream m_rhs("rhs");
    m_rhs << element_size << index;
    s_expression_ostream mul_idx("mul");
    mul_idx << m_lhs.expr() << m_rhs.expr() << get_debug_sexpr();
    s_expression_ostream shift((string_atom(shift_op)));
    s_expression_ostream s_lhs("lhs");
    s_lhs << base_offset;
    s_expression_ostream s_rhs("rhs");
    s_rhs << base_offset << offset_idx;
    shift << s_lhs.expr() << s_rhs.expr() << get_debug_sexpr();
    cfg.add_statement(mul_idx.expr());
    cfg.add_statement(shift.expr());
  } else {
    throw arbos_pass_error(
        "Unsupported type in "
        "GetElementPtr/ExtractValue/InsertValue/ExtractElement/InsertElement");
  }
}

s_expression GlobalReferences::translate_getelementptr(ARCode& cfg,
                                                       Instruction* gep) {
  const DataLayout& d = refs->getModuleDataLayout();

  s_expression base_address = get_value(cfg, gep->getOperand(0));
  const PointerType* ptr_type =
      dyn_cast< PointerType >(gep->getOperand(0)->getType());
  s_expression base_index = get_value(cfg, gep->getOperand(1));
  const Type* base_index_type = gep->getOperand(1)->getType();
  s_expression base_offset_i =
      create_var_sexpr(get_new_variable(), get_type_refid(base_index_type));
  s_expression base_offset =
      create_var_sexpr(get_new_variable(), get_type_refid(ptr_type));
  s_expression_ostream mul_lhs("lhs");
  mul_lhs << base_offset_i;
  s_expression k =
      generate_int_constant(d.getTypeAllocSize(ptr_type->getElementType()),
                            base_index_type);
  s_expression_ostream mul_rhs("rhs");
  mul_rhs << k << base_index;
  s_expression_ostream mul("mul");
  mul << mul_lhs.expr() << mul_rhs.expr() << get_debug_sexpr();
  s_expression_ostream base_shift("ptrshift");
  s_expression_ostream bs_lhs("lhs");
  bs_lhs << base_offset;
  s_expression_ostream bs_rhs("rhs");
  bs_rhs << base_address << base_offset_i;
  base_shift << bs_lhs.expr() << bs_rhs.expr() << get_debug_sexpr();
  cfg.add_statement(mul.expr());
  cfg.add_statement(base_shift.expr());

  gep_type_iterator gti = gep_type_begin(gep);
  ++gti;
  for (; gti != gep_type_end(gep); ++gti) {
    Value* idx = gti.getOperand();

    // Handle a struct index, which adds its field offset to the pointer.
    if (StructType* struct_ty = gti.getStructTypeOrNull()) {
      ConstantInt* idx_cst = dyn_cast< ConstantInt >(idx);

      if (!idx_cst) {
        throw arbos_pass_error(
            "Malformed GetElementPtr/ExtractValue/InsertValue (non-constant "
            "struct field index)");
      }

      z_number offset_i = d.getStructLayout(struct_ty)->getElementOffset(
          idx_cst->getZExtValue());
      s_expression offset = generate_int_constant(offset_i, idx->getType());
      s_expression_ostream shift((string_atom("ptrshift")));
      s_expression_ostream s_lhs("lhs");
      s_lhs << base_offset;
      s_expression_ostream s_rhs("rhs");
      s_rhs << base_offset << offset;
      shift << s_lhs.expr() << s_rhs.expr() << get_debug_sexpr();
      cfg.add_statement(shift.expr());
    } else {
      s_expression index = get_value(cfg, idx);
      const Type* idx_type = idx->getType();
      s_expression offset_idx =
          create_var_sexpr(get_new_variable(), get_type_refid(idx_type));
      s_expression_ostream m_lhs("lhs");
      m_lhs << offset_idx;
      s_expression element_size =
          generate_int_constant(d.getTypeAllocSize(gti.getIndexedType()),
                                idx_type);
      s_expression_ostream m_rhs("rhs");
      m_rhs << element_size << index;
      s_expression_ostream mul_idx("mul");
      mul_idx << m_lhs.expr() << m_rhs.expr() << get_debug_sexpr();
      s_expression_ostream shift((string_atom("ptrshift")));
      s_expression_ostream s_lhs("lhs");
      s_lhs << base_offset;
      s_expression_ostream s_rhs("rhs");
      s_rhs << base_offset << offset_idx;
      shift << s_lhs.expr() << s_rhs.expr() << get_debug_sexpr();
      cfg.add_statement(mul_idx.expr());
      cfg.add_statement(shift.expr());
    }
  }

  return gen_sexpr_assign(get_value(cfg, gep), base_offset);
}

s_expression GlobalReferences::translate_extractvalue(ARCode& cfg,
                                                      ExtractValueInst* ev) {
  s_expression_ostream o("extractelem");
  Value* aggregate = ev->getAggregateOperand();
  Type* ty = aggregate->getType();
  IntegerType* idx_type = IntegerType::get(*_llvm_context, 64);
  s_expression base_offset =
      create_var_sexpr(get_new_variable(), get_type_refid(idx_type));

  s_expression init =
      gen_sexpr_assign(base_offset, generate_zero_constant(idx_type));
  cfg.add_statement(init);
  for (ExtractValueInst::idx_iterator idx_it = ev->idx_begin();
       idx_it != ev->idx_end();
       ++idx_it) {
    CompositeType* aggregate_type =
        const_cast< CompositeType* >(dyn_cast< CompositeType >(ty));
    unsigned int idx = *idx_it;
    Value* idx_value = ConstantInt::get(idx_type, idx);
    translate_index(cfg, aggregate_type, idx_value, base_offset, "add");
    ty = aggregate_type->getTypeAtIndex(idx);
  }

  o << get_value(cfg, ev) << get_value(cfg, aggregate) << base_offset
    << get_debug_sexpr();
  return o.expr();
}

s_expression GlobalReferences::translate_insertvalue(ARCode& cfg,
                                                     InsertValueInst* iv) {
  s_expression_ostream o("insertelem");
  Value* aggregate = iv->getAggregateOperand();
  Value* element = iv->getInsertedValueOperand();
  Type* ty = aggregate->getType();
  Type* idx_type = IntegerType::get(*_llvm_context, 64);
  s_expression base_offset =
      create_var_sexpr(get_new_variable(), get_type_refid(idx_type));

  s_expression init =
      gen_sexpr_assign(base_offset, generate_zero_constant(idx_type));
  cfg.add_statement(init);
  for (InsertValueInst::idx_iterator idx_it = iv->idx_begin();
       idx_it != iv->idx_end();
       ++idx_it) {
    CompositeType* aggregate_type = dyn_cast< CompositeType >(ty);
    unsigned int idx = *idx_it;
    Value* idx_value = ConstantInt::get(idx_type, idx);
    translate_index(cfg, aggregate_type, idx_value, base_offset, "add");
    ty = aggregate_type->getTypeAtIndex(idx);
  }

  o << get_value(cfg, iv) << get_value(cfg, aggregate)
    << get_value(cfg, element) << base_offset << get_debug_sexpr();
  return o.expr();
}

s_expression GlobalReferences::translate_extractelement(
    ARCode& cfg, ExtractElementInst* ee) {
  const DataLayout& d = refs->getModuleDataLayout();
  Value* vector = ee->getVectorOperand();
  VectorType* tvec = dyn_cast< VectorType >(vector->getType());
  z_number elem_size = d.getTypeAllocSize(tvec->getElementType());
  Value* idx_value = ee->getIndexOperand();
  IntegerType* idx_type = IntegerType::get(*_llvm_context, 64);

  // generate offset = elem_size * idx_value
  s_expression offset =
      create_var_sexpr(get_new_variable(), get_type_refid(idx_type));
  s_expression_ostream lhs("lhs");
  lhs << offset;
  s_expression_ostream rhs("rhs");
  rhs << generate_int_constant(elem_size, idx_type)
      << get_value(cfg, idx_value);
  s_expression_ostream mul("mul");
  mul << lhs.expr() << rhs.expr() << get_debug_sexpr();
  cfg.add_statement(mul.expr());

  s_expression_ostream o("extractelem");
  o << get_value(cfg, ee) << get_value(cfg, vector) << offset
    << get_debug_sexpr();
  return o.expr();
}

s_expression GlobalReferences::translate_insertelement(ARCode& cfg,
                                                       InsertElementInst* ie) {
  const DataLayout& d = refs->getModuleDataLayout();
  Value* vector = ie->getOperand(0);
  VectorType* tvec = dyn_cast< VectorType >(vector->getType());
  z_number elem_size = d.getTypeAllocSize(tvec->getElementType());
  Value* element = ie->getOperand(1);
  Value* idx_value = ie->getOperand(2);
  IntegerType* idx_type = IntegerType::get(*_llvm_context, 64);

  // generate offset = elem_size * idx_value
  s_expression offset =
      create_var_sexpr(get_new_variable(), get_type_refid(idx_type));
  s_expression_ostream lhs("lhs");
  lhs << offset;
  s_expression_ostream rhs("rhs");
  rhs << generate_int_constant(elem_size, idx_type)
      << get_value(cfg, idx_value);
  s_expression_ostream mul("mul");
  mul << lhs.expr() << rhs.expr() << get_debug_sexpr();
  cfg.add_statement(mul.expr());

  s_expression_ostream o("insertelem");
  o << get_value(cfg, ie) << get_value(cfg, vector) << get_value(cfg, element)
    << offset << get_debug_sexpr();
  return o.expr();
}

s_expression GlobalReferences::translate_constant_local_addr(ARCode& cfg,
                                                             Value* v) {
  s_expression_ostream out("cst");
  s_expression_ostream k("localvariableref");
  s_expression_ostream name("name");
  name << string_atom(v->getName().str());
  k << name.expr();
  k << create_ty_ref_sexpr(get_type_refid(v->getType()));
  out << k.expr();
  return out.expr();
}

s_expression GlobalReferences::translate_constant(ARCode& cfg, Constant* cst) {
  const DataLayout& d = refs->getModuleDataLayout();

  s_expression_ostream out("cst");
  if (dyn_cast< BlockAddress >(cst)) {
    throw arbos_pass_error(
        "Labels as values are not supported: constant 'blockaddress' is not "
        "allowed in bitcode");
  } else if (dyn_cast< ConstantAggregateZero >(cst)) {
    return generate_zero_constant(cst->getType());
  } else if (ConstantDataArray* array = dyn_cast< ConstantDataArray >(cst)) {
    const ArrayType* tarray = dyn_cast< ArrayType >(cst->getType());
    Type* telement = const_cast< Type* >(tarray->getElementType());
    z_number elem_size = d.getTypeAllocSize(telement);
    s_expression_ostream k("range");
    s_expression_ostream values("values");
    for (unsigned int i = 0; i < array->getNumElements(); i++) {
      Constant* c = array->getElementAsConstant(i);
      s_expression_ostream v_s("value");
      v_s << z_number_atom(z_number::from_ulong(i) * elem_size)
          << translate_constant(cfg, c);
      values << v_s.expr();
    }
    k << values.expr();
    k << create_ty_ref_sexpr(get_type_refid(cst->getType()));
    out << k.expr();
  } else if (ConstantDataVector* vec = dyn_cast< ConstantDataVector >(cst)) {
    const VectorType* tvec = dyn_cast< VectorType >(cst->getType());
    Type* telement = const_cast< Type* >(tvec->getElementType());
    z_number elem_size = d.getTypeAllocSize(telement);
    s_expression_ostream k("range");
    s_expression_ostream values("values");
    for (unsigned int i = 0; i < vec->getNumElements(); i++) {
      Constant* c = vec->getElementAsConstant(i);
      s_expression_ostream v_s("value");
      v_s << z_number_atom(z_number::from_ulong(i) * elem_size)
          << translate_constant(cfg, c);
      values << v_s.expr();
    }
    k << values.expr();
    k << create_ty_ref_sexpr(get_type_refid(cst->getType()));
    out << k.expr();
  } else if (dyn_cast< ConstantStruct >(cst)) {
    StructType* tstruct =
        const_cast< StructType* >(dyn_cast< StructType >(cst->getType()));
    s_expression_ostream k("range");
    s_expression_ostream values("values");
    Constant::op_iterator p = cst->op_begin(), e = cst->op_end();
    for (unsigned int i = 0; p != e; ++i, ++p) {
      Constant* c = dyn_cast< Constant >(*p);
      s_expression_ostream v_s("value");
      z_number offset = d.getStructLayout(tstruct)->getElementOffset(i);
      v_s << z_number_atom(offset) << translate_constant(cfg, c);
      values << v_s.expr();
    }
    k << values.expr();
    k << create_ty_ref_sexpr(get_type_refid(cst->getType()));
    out << k.expr();
  } else if (dyn_cast< ConstantArray >(cst)) {
    const ArrayType* tarray = dyn_cast< ArrayType >(cst->getType());
    Type* telement = const_cast< Type* >(tarray->getElementType());
    z_number elem_size = d.getTypeAllocSize(telement);
    s_expression_ostream k("range");
    s_expression_ostream values("values");
    Constant::op_iterator p = cst->op_begin(), e = cst->op_end();
    for (unsigned int idx = 0; p != e; ++idx, ++p) {
      Constant* c = dyn_cast< Constant >(*p);
      s_expression_ostream v_s("value");
      v_s << z_number_atom(z_number::from_ulong(idx) * elem_size)
          << translate_constant(cfg, c);
      values << v_s.expr();
    }
    k << values.expr();
    k << create_ty_ref_sexpr(get_type_refid(cst->getType()));
    out << k.expr();
  } else if (dyn_cast< ConstantVector >(cst)) {
    const VectorType* tvec = dyn_cast< VectorType >(cst->getType());
    Type* telement = const_cast< Type* >(tvec->getElementType());
    z_number elem_size = d.getTypeAllocSize(telement);
    s_expression_ostream k("range");
    s_expression_ostream values("values");
    Constant::op_iterator p = cst->op_begin(), e = cst->op_end();
    for (unsigned int idx = 0; p != e; ++idx, ++p) {
      Constant* c = dyn_cast< Constant >(*p);
      s_expression_ostream v_s("value");
      v_s << z_number_atom(z_number::from_ulong(idx) * elem_size)
          << translate_constant(cfg, c);
      values << v_s.expr();
    }
    k << values.expr();
    k << create_ty_ref_sexpr(get_type_refid(cst->getType()));
    out << k.expr();
  } else if (ConstantExpr* e = dyn_cast< ConstantExpr >(cst)) {
    switch (e->getOpcode()) {
      // Cast Operations
      case Instruction::Trunc:
      case Instruction::ZExt:
      case Instruction::SExt:
      case Instruction::FPToUI:
      case Instruction::FPToSI:
      case Instruction::UIToFP:
      case Instruction::SIToFP:
      case Instruction::FPTrunc:
      case Instruction::FPExt:
      case Instruction::PtrToInt:
      case Instruction::IntToPtr:
      case Instruction::BitCast:
      // Binary and bitwise unary operations
      case Instruction::Add:
      case Instruction::FAdd:
      case Instruction::Sub:
      case Instruction::FSub:
      case Instruction::Mul:
      case Instruction::FMul:
      case Instruction::UDiv:
      case Instruction::SDiv:
      case Instruction::FDiv:
      case Instruction::URem:
      case Instruction::SRem:
      case Instruction::FRem:
      case Instruction::Shl:
      case Instruction::LShr:
      case Instruction::AShr:
      case Instruction::And:
      case Instruction::Or:
      case Instruction::Xor: {
        std::string opcodename = Instruction::getOpcodeName(e->getOpcode());
        s_expression_ostream op((string_atom(opcodename)));
        s_expression ret_var =
            create_var_sexpr(get_new_variable(), get_type_refid(e->getType()));

        s_expression_ostream lhs("lhs");
        lhs << ret_var;

        s_expression_ostream rhs("rhs");
        for (unsigned int i = 0; i < e->getNumOperands(); ++i) {
          Constant* op_cst = dyn_cast< Constant >(e->getOperand(i));
          s_expression op_val = translate_constant(cfg, op_cst);
          rhs << op_val;
        }

        op << lhs.expr() << rhs.expr() << get_debug_sexpr();
        cfg.add_statement(op.expr());
        return ret_var;
      } break;

      // Boolean operations
      case Instruction::FCmp:
      case Instruction::ICmp: {
        CmpInst* cmp = CmpInst::Create((Instruction::OtherOps)e->getOpcode(),
                                       static_cast< CmpInst::Predicate >(
                                           e->getPredicate()),
                                       e->getOperand(0),
                                       e->getOperand(1));
        cfg.split_blocks(cmp, cmp);
        s_expression ret_val = get_value(cfg, cmp);
        delete cmp;
        return ret_val;
      } break;

      case Instruction::Select: {
        SelectInst* select = SelectInst::Create(e->getOperand(0),
                                                e->getOperand(1),
                                                e->getOperand(2));
        cfg.split_blocks(select, select->getCondition());
        s_expression ret_val = get_value(cfg, select);
        delete select;
        return ret_val;
      } break;

      case Instruction::GetElementPtr: {
        std::vector< Value* > indices;
        for (unsigned int i = 1; i < e->getNumOperands(); ++i) {
          indices.push_back(e->getOperand(i));
        }

        Instruction* inst =
            GetElementPtrInst::Create(cast< PointerType >(e->getOperand(0)
                                                              ->getType()
                                                              ->getScalarType())
                                          ->getElementType(),
                                      e->getOperand(0),
                                      indices);
        cfg.add_statement(translate_getelementptr(cfg, inst));
        s_expression ret_val = get_value(cfg, inst);
        delete inst;
        return ret_val;
      } break;

      case Instruction::ExtractValue: {
        ArrayRef< unsigned int > indices = e->getIndices();
        ExtractValueInst* inst =
            ExtractValueInst::Create(e->getOperand(0), indices);
        cfg.add_statement(translate_extractvalue(cfg, inst));
        s_expression ret_val = get_value(cfg, inst);
        delete inst;
        return ret_val;
      } break;

      case Instruction::InsertValue: {
        ArrayRef< unsigned int > indices = e->getIndices();
        InsertValueInst* inst = InsertValueInst::Create(e->getOperand(0),
                                                        e->getOperand(1),
                                                        indices);
        cfg.add_statement(translate_insertvalue(cfg, inst));
        s_expression ret_val = get_value(cfg, inst);
        delete inst;
        return ret_val;
      } break;

      case Instruction::ExtractElement: {
        ExtractElementInst* inst =
            ExtractElementInst::Create(e->getOperand(0), e->getOperand(1));
        cfg.add_statement(translate_extractelement(cfg, inst));
        s_expression ret_val = get_value(cfg, inst);
        delete inst;
        return ret_val;
      } break;

      case Instruction::InsertElement: {
        InsertElementInst* inst = InsertElementInst::Create(e->getOperand(0),
                                                            e->getOperand(1),
                                                            e->getOperand(2));
        cfg.add_statement(translate_insertelement(cfg, inst));
        s_expression ret_val = get_value(cfg, inst);
        delete inst;
        return ret_val;
      } break;

      default: {
        throw arbos_pass_error(
            "Internal error: unrecognized constant expression");
      }
    }
  } else if (ConstantFP* x = dyn_cast< ConstantFP >(cst)) {
    s_expression_ostream k("constantfp");
    APFloat val = x->getValueAPF();
    char buf[256];
    val.convertToHexString(buf, 0, false, APFloat::rmNearestTiesToEven);
    s_expression_ostream v("val");
    v << fp_number_atom(fp_number(buf));
    k << v.expr();
    k << create_ty_ref_sexpr(get_type_refid(cst->getType()));
    out << k.expr();
  } else if (ConstantInt* x = dyn_cast< ConstantInt >(cst)) {
    s_expression_ostream k("constantint");
    s_expression_ostream value_s("val");
    value_s << z_number_atom(x->getValue().toString(10, true));
    k << value_s.expr();
    k << create_ty_ref_sexpr(get_type_refid(cst->getType()));
    out << k.expr();
  } else if (dyn_cast< ConstantPointerNull >(cst)) {
    s_expression_ostream k("constantpointernull");
    k << create_ty_ref_sexpr(get_type_refid(cst->getType()));
    out << k.expr();
  } else if (Function* x = dyn_cast< Function >(cst)) {
    s_expression_ostream k("constantfunctionptr");
    k << create_name_sexpr(x->getName().str());
    k << create_ty_ref_sexpr(get_type_refid(x->getFunctionType()));
    out << k.expr();
  } else if (GlobalAlias* alias = dyn_cast< GlobalAlias >(cst)) {
    return translate_constant(cfg, alias->getAliasee());
  } else if (dyn_cast< GlobalVariable >(cst)) {
    // Global variables are identified in identify_global_variables
    s_expression_ostream k("globalvariableref");
    s_expression_ostream name("name");
    name << string_atom(cst->getName().str());
    k << name.expr();
    s_expression_ostream gv("gv");
    gv << index64_atom(_global_vars[dyn_cast< GlobalVariable >(cst)]->get_id());
    k << gv.expr();
    k << create_ty_ref_sexpr(get_type_refid(cst->getType()));
    out << k.expr();
  } else if (dyn_cast< UndefValue >(cst)) {
    s_expression_ostream k("undefined");
    k << create_ty_ref_sexpr(get_type_refid(cst->getType()));
    out << k.expr();
  } else {
    throw arbos_pass_error("Unsupported type in constant");
  }
  return out.expr();
}

s_expression GlobalReferences::translate_type(const Type* t) {
  const DataLayout& d = refs->getModuleDataLayout();
  s_expression_ostream s("type");

  if (IntegerType* tint =
          const_cast< IntegerType* >(dyn_cast< IntegerType >(t))) {
    s_expression_ostream i("int");
    append_type_sizes(i, tint->getBitWidth(), d.getTypeStoreSize(tint));
    s << i.expr();
  } else if (FunctionType* tfunc =
                 const_cast< FunctionType* >(dyn_cast< FunctionType >(t))) {
    s_expression_ostream f("function"), retty("return_ty"), params("params_ty"),
        isVA("isVarArg");

    f << (retty << index64_atom((get_type_refid(tfunc->getReturnType()))))
             .expr();

    for (unsigned int i = 0; i < tfunc->getNumParams(); i++) {
      params << index64_atom(get_type_refid(tfunc->getParamType(i)));
    }
    f << params.expr();

    isVA << string_atom((tfunc->isVarArg() ? "True" : "False"));
    f << isVA.expr();

    s << f.expr();
  } else if (StructType* tstruct =
                 const_cast< StructType* >(dyn_cast< StructType >(t))) {
    if (tstruct->isOpaque()) {
      s_expression_ostream o("opaque");
      s << o.expr();
    } else {
      s_expression_ostream st("struct");

      s_expression_ostream s_layout("layout");
      for (unsigned int i = 0; i < t->getNumContainedTypes(); i++) {
        s_expression_ostream l(
            z_number_atom(d.getStructLayout(tstruct)->getElementOffset(i)));
        l << index64_atom(get_type_refid(t->getContainedType(i)));
        s_layout << l.expr();
      }
      st << s_layout.expr();

      s_expression_ostream s_align("align");
      s_align << z_number_atom(d.getStructLayout(tstruct)->getAlignment() * 8);
      st << s_align.expr();

      append_type_sizes(st,
                        d.getStructLayout(tstruct)->getSizeInBits(),
                        d.getTypeStoreSize(tstruct));

      s << st.expr();
    }
  } else if (ArrayType* tarray =
                 const_cast< ArrayType* >(dyn_cast< ArrayType >(t))) {
    s_expression_ostream a("array");

    s_expression_ostream len("len");
    len << z_number_atom(
        z_number::from_ulong((unsigned long)tarray->getNumElements()));
    a << len.expr();

    s_expression_ostream s_contained_types("ty");
    for (unsigned int i = 0; i < t->getNumContainedTypes(); i++) {
      s_contained_types << index64_atom(get_type_refid(t->getContainedType(i)));
    }
    a << s_contained_types.expr();

    append_type_sizes(a,
                      d.getTypeSizeInBits(tarray),
                      d.getTypeStoreSize(tarray));

    s << a.expr();
  } else if (VectorType* tvec =
                 const_cast< VectorType* >(dyn_cast< VectorType >(t))) {
    s_expression_ostream a("array");

    s_expression_ostream len("len");
    len << z_number_atom(
        z_number::from_ulong((unsigned long)tvec->getNumElements()));
    a << len.expr();

    s_expression_ostream s_contained_types("ty");
    for (unsigned int i = 0; i < t->getNumContainedTypes(); i++) {
      s_contained_types << index64_atom(get_type_refid(t->getContainedType(i)));
    }
    a << s_contained_types.expr();

    append_type_sizes(a, d.getTypeSizeInBits(tvec), d.getTypeStoreSize(tvec));

    s << a.expr();
  } else if (PointerType* tptr =
                 const_cast< PointerType* >(dyn_cast< PointerType >(t))) {
    s_expression_ostream p("ptr");

    s_expression_ostream s_contained_types("ty");
    for (unsigned int i = 0; i < t->getNumContainedTypes(); i++) {
      s_contained_types << index64_atom(get_type_refid(t->getContainedType(i)));
    }
    p << s_contained_types.expr();

    append_type_sizes(p, d.getTypeSizeInBits(tptr), d.getTypeStoreSize(tptr));

    s << p.expr();
  } else if (t->isFloatingPointTy()) {
    s_expression_ostream f("float");

    if (t->getPrimitiveSizeInBits() == 16) {
      f << string_atom("half");
    } else if (t->isFloatTy()) {
      f << string_atom("float");
    } else if (t->isDoubleTy()) {
      f << string_atom("double");
    } else if (t->isX86_FP80Ty()) {
      f << string_atom("x86_fp80");
    } else if (t->isFP128Ty()) {
      f << string_atom("fp128");
    } else if (t->isPPC_FP128Ty()) {
      f << string_atom("ppc_fp128");
    } else {
      throw arbos_pass_error("Unknown floating-point type found");
    }

    append_type_sizes(f,
                      t->getPrimitiveSizeInBits(),
                      d.getTypeStoreSize(const_cast< Type* >(t)));

    s << f.expr();
  } else if (t->isLabelTy()) {
    throw arbos_pass_error("Labels as values are not supported");
  } else if (t->isVoidTy()) {
    s_expression_ostream v("void");

    append_type_sizes(v, 0, 0);

    s << v.expr();
  } else {
    throw arbos_pass_error("Unknown LLVM type found");
  }

  return s.expr();
}

void GlobalReferences::print_files(std::ostream& outf) {
  std::unordered_map< std::string, uint64_t >::iterator p = _files.begin();
  for (; p != _files.end(); p++) {
    s_expression_ostream d("def");
    s_expression_ostream s(index64_atom(p->second));
    s_expression_ostream f("file");
    f << string_atom(p->first);
    s << f.expr();
    d << s.expr();
    outf << d.expr() << std::endl;
  }
  _files.clear();
}

void GlobalReferences::print_global_vars(std::ostream& outf) {
  std::unordered_map< GlobalVariable*,
                      std::shared_ptr< ARGlobalVar > >::iterator p =
      _global_vars.begin();
  for (; p != _global_vars.end(); p++) {
    std::shared_ptr< ARGlobalVar > gv = p->second;
    s_expression_ostream d("def");
    s_expression_ostream s(index64_atom(gv->get_id()));
    s << gv->get_definition();
    d << s.expr();
    s_expression_formatter fmt(outf);
    fmt << sexpr_indent("def");
    fmt << sexpr_indent("code");
    fmt << sexpr_indent("basicblocks");
    fmt << sexpr_indent("basicblock");
    fmt << sexpr_indent("instructions");
    fmt << sexpr_indent("trans");
    fmt << d.expr();
    outf << std::endl;
  }
  _global_vars.clear();
}

void GlobalReferences::print_data_types(std::ostream& outf) {
  std::unordered_map< const Type*, std::shared_ptr< ARType > >::iterator p =
      _data_types.begin();
  for (; p != _data_types.end(); p++) {
    s_expression_ostream d("def");
    s_expression_ostream s(index64_atom(p->second->get_id()));
    s << p->second->get_description();
    d << s.expr();
    outf << d.expr() << std::endl;
  }
  _data_types.clear();
}

void GlobalReferences::printAR(std::ostream& outf) {
  print_files(outf);
  print_data_types(outf);
  print_global_vars(outf);
}

void GlobalReferences::clear() {
  _files.clear();
  _data_types.clear();
  _global_vars.clear();
  reset_debug_info();
}

/**
 * Common translation operations
 */

// Reduce the number of cases. After calling this function only six
// cases: EQ, NEQ, SLE, ULE, ULT, and SLT
ICmpInst* normalizeICmpInst(ICmpInst* I) {
  switch (I->getPredicate()) {
    case ICmpInst::ICMP_UGT:
    case ICmpInst::ICMP_SGT:
      I->swapOperands();
      break;
    case ICmpInst::ICMP_UGE:
    case ICmpInst::ICMP_SGE:
      I->swapOperands();
      break;
    default:;
  }
  return I;
}

FCmpInst* normalizeFCmpInst(FCmpInst* I) {
  switch (I->getPredicate()) {
    case CmpInst::FCMP_OGT:
    case CmpInst::FCMP_OGE:
    case CmpInst::FCMP_UGT:
    case CmpInst::FCMP_UGE:
      I->swapOperands();
      break;
    default:;
  }
  return I;
}

s_expression gen_assign(ARCode& cfg, Value* lhs, Value* rhs) {
  return gen_sexpr_assign(refs->get_value(cfg, lhs), refs->get_value(cfg, rhs));
}

s_expression gen_assertion(ARCode& cfg, Value* v, bool condition) {
  s_expression_ostream s("assert");

  s_expression_ostream k("kind");
  k << string_atom("i");
  s << k.expr();

  s_expression_ostream cmp("cmp");

  s_expression_ostream c("cond");
  c << string_atom(condition ? "true" : "false");
  cmp << c.expr();

  s_expression_ostream p("pred");
  p << string_atom("eq");
  cmp << p.expr();

  s_expression_ostream s_ops("ops");
  s_ops << refs->get_value(cfg, v); // the variable
  s_ops << refs->get_value(cfg,
                           ConstantInt::get(v->getType(),
                                            1)); // the constant value
  cmp << s_ops.expr();

  s << cmp.expr() << refs->get_debug_sexpr();

  return s.expr();
}

s_expression gen_assertion(ARCode& cfg, CmpInst* inst, bool condition) {
  if (inst->getOpcode() == Instruction::ICmp) {
    inst = normalizeICmpInst(dyn_cast< ICmpInst >(inst));
  } else { // Instruction::FCmp
    inst = normalizeFCmpInst(dyn_cast< FCmpInst >(inst));
  }

  std::string opcodename = Instruction::getOpcodeName(inst->getOpcode());
  CmpInst* ci = dyn_cast< CmpInst >(inst);
  CmpInst::Predicate pred = ci->getPredicate();
  std::string predstr;

  switch (pred) {
    case CmpInst::FCMP_FALSE: ///< 0 0 0 0    Always false (always folded)
      predstr = "false";
      break;

    case CmpInst::FCMP_OEQ: ///< 0 0 0 1    True if ordered and equal
      predstr = "oeq";
      break;

    case CmpInst::FCMP_OGT: ///< 0 0 1 0    True if ordered and greater than
      predstr = "ogt";
      break;

    case CmpInst::FCMP_OGE: ///< 0 0 1 1    True if ordered and greater than or
      /// equal
      predstr = "oge";
      break;

    case CmpInst::FCMP_OLT: ///< 0 1 0 0    True if ordered and less than
      predstr = "olt";
      break;

    case CmpInst::FCMP_OLE: ///< 0 1 0 1    True if ordered and less than or
      /// equal
      predstr = "ole";
      break;

    case CmpInst::FCMP_ONE: ///< 0 1 1 0    True if ordered and operands are
      /// unequal
      predstr = "one";
      break;

    case CmpInst::FCMP_ORD: ///< 0 1 1 1    True if ordered (no nans)
      predstr = "ord";
      break;

    case CmpInst::FCMP_UNO: ///< 1 0 0 0    True if unordered: isnan(X) |
      /// isnan(Y)
      predstr = "uno";
      break;

    case CmpInst::FCMP_UEQ: ///< 1 0 0 1    True if unordered or equal
      predstr = "ueq";
      break;

    case CmpInst::FCMP_UGT: ///< 1 0 1 0    True if unordered or greater than
      predstr = "ugt";
      break;

    case CmpInst::FCMP_UGE: ///< 1 0 1 1    True if unordered, greater than, or
      /// equal
      predstr = "uge";
      break;

    case CmpInst::FCMP_ULT: ///< 1 1 0 0    True if unordered or less than
      predstr = "ult";
      break;

    case CmpInst::FCMP_ULE: ///< 1 1 0 1    True if unordered, less than, or
      /// equal
      predstr = "ule";
      break;

    case CmpInst::FCMP_UNE: ///< 1 1 1 0    True if unordered or not equal
      predstr = "une";
      break;

    case CmpInst::FCMP_TRUE: ///< 1 1 1 1    Always true (always folded)
      predstr = "true";
      break;

    case CmpInst::ICMP_EQ: ///< equal
      predstr = "eq";
      break;

    case CmpInst::ICMP_NE: ///< not equal
      predstr = "ne";
      break;

    case CmpInst::ICMP_UGT: ///< unsigned greater than
      predstr = "ugt";
      break;

    case CmpInst::ICMP_UGE: ///< unsigned greater or equal
      predstr = "uge";
      break;

    case CmpInst::ICMP_ULT: ///< unsigned less than
      predstr = "ult";
      break;

    case CmpInst::ICMP_ULE: ///< unsigned less or equal
      predstr = "ule";
      break;

    case CmpInst::ICMP_SGT: ///< signed greater than
      predstr = "sgt";
      break;

    case CmpInst::ICMP_SGE: ///< signed greater or equal
      predstr = "sge";
      break;

    case CmpInst::ICMP_SLT: ///< signed less than
      predstr = "slt";
      break;

    case CmpInst::ICMP_SLE: ///< signed less or equal
      predstr = "sle";
      break;

    default:
      // If the bitcode is consistent this case shall never be reached
      throw arbos_pass_error(
          "Unrecognized predicate encountered whilst translating icmp/fcmp");
  }

  s_expression_ostream s("assert");
  s_expression_ostream k("kind");
  k << string_atom(inst->isFPPredicate() ? "fp" : "i");
  s << k.expr();

  s_expression_ostream cmp("cmp");

  s_expression_ostream c("cond");
  c << string_atom(condition ? "true" : "false");
  cmp << c.expr();

  s_expression_ostream p("pred");
  p << string_atom(predstr);
  cmp << p.expr();

  s_expression_ostream s_ops("ops");
  for (unsigned int i = 0; i < inst->getNumOperands(); i++) {
    s_ops << refs->get_value(cfg, inst->getOperand(i));
  }
  cmp << s_ops.expr();

  s << cmp.expr() << refs->get_debug_sexpr();
  return s.expr();
}

/**
 * ARCode implementation
 */

void ARCode::enter_basic_block(std::string block) {
  _current_ar_bblock = block;
  _bblocks[_current_ar_bblock];
}

std::string ARCode::get_current_block() {
  return _current_ar_bblock;
}

void ARCode::add_statement(s_expression_ref c) {
  if (c) {
    _bblocks[_current_ar_bblock].push_back(*c);
  }
}

void ARCode::add_transition(std::string src, std::string dest) {
  std::vector< std::string >& destinations = _transitions[src];
  if (destinations.end() ==
      find(destinations.begin(), destinations.end(), dest)) {
    _transitions[src].push_back(dest);
  }
}

void ARCode::remove_transition(std::string src, std::string dest) {
  std::vector< std::string >::iterator p =
      find(_transitions[src].begin(), _transitions[src].end(), dest);
  if (p != _transitions[src].end()) {
    _transitions[src].erase(p);
  }
}

std::string ARCode::get_new_destination(std::string orig_src,
                                        std::string orig_dest) {
  std::string new_dest = "*in_" + orig_src + "_to_" + orig_dest;
  std::unordered_map< std::string, std::vector< s_expression_ref > >::iterator
      p = _bblocks.begin();
  for (; p != _bblocks.end(); p++) {
    if (p->first.find(new_dest) == 0) {
      return p->first;
    }
  }
  return orig_dest;
}

std::string ARCode::get_new_source(std::string orig_src,
                                   std::string orig_dest) {
  // Case 1. Check for new source coming out of a conditional br.
  std::string new_src = "*out_" + orig_src + "_to_" + orig_dest;
  std::unordered_map< std::string, std::vector< s_expression_ref > >::iterator
      p = _bblocks.begin();
  for (; p != _bblocks.end(); p++) {
    if (p->first.find(new_src) == 0) {
      return p->first;
    }
  }

  // Case 2. Check for new source coming out of an unconditional br
  // in a merger of more than one split basic block.
  new_src = "*out_" + orig_src + "_merge_";
  p = _bblocks.begin();
  for (; p != _bblocks.end(); p++) {
    if (p->first.find(new_src) == 0) {
      return p->first;
    }
  }

  // Case 3. No new sources.
  return orig_src;
}

void ARCode::route_to_new_dest(std::string orig_src, std::string orig_dest) {
  add_transition(_current_ar_bblock, get_new_destination(orig_src, orig_dest));
}

void ARCode::split_blocks(Instruction* inst, Value* assertion) {
  std::string current_llvm_bblock = inst->getParent()->getName().str();
  std::string opcodename = Instruction::getOpcodeName(inst->getOpcode());
  std::string splitct_str = get_next_split_ct(current_llvm_bblock, opcodename);

  std::string split_true = "*_" + current_llvm_bblock + "_split_" +
                           splitct_str + opcodename + "_true";
  std::string split_false = "*_" + current_llvm_bblock + "_split_" +
                            splitct_str + opcodename + "_false";

  std::string merge_bblock =
      "*out_" + current_llvm_bblock + "_merge_" + splitct_str + opcodename;

  // transfer all outgoing edges from current LLVM bblock to the ar converge
  // bblock in
  std::vector< std::string > destinations = _transitions[_current_ar_bblock];
  _transitions[_current_ar_bblock].clear();
  _transitions[merge_bblock] = destinations;
  add_transition(_current_ar_bblock, split_true);
  add_transition(_current_ar_bblock, split_false);
  add_transition(split_true, merge_bblock);
  add_transition(split_false, merge_bblock);

  CmpInst* cmp = dyn_cast< CmpInst >(inst);

  boost::optional< s_expression > assert_true;
  if (cmp) {
    assert_true = gen_assertion(*this, cmp, true);
  } else if (dyn_cast< SelectInst >(inst)) {
    cmp = dyn_cast< CmpInst >(assertion);
    assert_true = cmp ? gen_assertion(*this, cmp, true)
                      : gen_assertion(*this, assertion, true);
  }
  assert(assert_true && "Cannot generate assertion true");

  enter_basic_block(split_true);
  add_statement(assert_true);
  if (dyn_cast< SelectInst >(inst)) {
    add_statement(gen_assign(*this, inst, inst->getOperand(1)));
  } else {
    add_statement(
        gen_assign(*this, inst, ConstantInt::getTrue(assertion->getType())));
  }

  boost::optional< s_expression > assert_false;
  if (cmp) {
    assert_false = gen_assertion(*this, cmp, false);
  } else if (dyn_cast< SelectInst >(inst)) {
    cmp = dyn_cast< CmpInst >(assertion);
    assert_false = cmp ? gen_assertion(*this, cmp, false)
                       : gen_assertion(*this, assertion, false);
  }
  assert(assert_false && "Cannot generate assertion false");

  enter_basic_block(split_false);
  add_statement(assert_false);
  if (dyn_cast< SelectInst >(inst)) {
    add_statement(gen_assign(*this, inst, inst->getOperand(2)));
  } else {
    add_statement(
        gen_assign(*this, inst, ConstantInt::getFalse(assertion->getType())));
  }

  enter_basic_block(merge_bblock);
}

s_expression ARCode::expr() {
  s_expression_ostream code("code");

  code << (s_expression_ostream("entry") << string_atom(_entry_block)).expr();

  if (!_exit_block.empty()) {
    code << (s_expression_ostream("exit") << string_atom(_exit_block)).expr();
  } else {
    code << s_expression_ostream("exit").expr();
  }

  if (!_unreachable_block.empty()) {
    code << (s_expression_ostream("unreachable")
             << string_atom(_unreachable_block))
                .expr();
  } else {
    code << s_expression_ostream("unreachable").expr();
  }

  if (!_resume_block.empty()) {
    code << (s_expression_ostream("ehresume") << string_atom(_resume_block))
                .expr();
  } else {
    code << s_expression_ostream("ehresume").expr();
  }

  std::unordered_map< std::string,
                      std::vector< boost::optional< s_expression > > >::iterator
      bb = _bblocks.begin();
  s_expression_ostream s_bbs("basicblocks");
  for (; bb != _bblocks.end(); bb++) {
    s_expression_ostream s_bb("basicblock");
    s_expression_ostream n("name");
    n << string_atom(bb->first);
    s_bb << n.expr();

    // Emit instructions
    s_expression_ostream instructions("instructions");
    std::vector< boost::optional< s_expression > >::iterator i =
        bb->second.begin();
    for (; i != bb->second.end(); i++) {
      instructions << **i;
    }
    s_bb << instructions.expr();
    s_bbs << s_bb.expr();
  }
  code << s_bbs.expr();

  std::unordered_map< std::string, std::vector< std::string > >::iterator t =
      _transitions.begin();
  s_expression_ostream s_trans("trans");
  for (; t != _transitions.end(); t++) {
    std::string src = t->first;
    std::vector< std::string >::iterator dest = t->second.begin();
    for (; dest != t->second.end(); dest++) {
      s_expression_ostream e("edge");
      e << string_atom(src) << string_atom(*dest);
      s_trans << e.expr();
    }
  }
  code << s_trans.expr();

  return code.expr();
}

void ARCode::printDOT(std::ostream& out) {
  typedef boost::adjacency_list<
      boost::vecS,
      boost::vecS,
      boost::directedS,
      boost::property< boost::vertex_color_t, boost::default_color_type >,
      boost::property< boost::edge_weight_t, int > >
      Graph; // Assign vertex id to each bblock

  std::unordered_map< std::string, int > bblock_to_descriptor;
  bblock_to_descriptor.reserve(_bblocks.size());

  std::vector< const char* > vertex_labels;
  vertex_labels.reserve(_bblocks.size());

  std::unordered_map< std::string,
                      std::vector< boost::optional< s_expression > > >::iterator
      bb = _bblocks.begin();

  for (int index = 0; bb != _bblocks.end(); bb++) {
    bblock_to_descriptor[bb->first] = index;
    vertex_labels.push_back(bb->first.c_str());
    index++;
  }

  typedef std::pair< int, int > Edge;
  std::vector< Edge > edges;
  std::unordered_map< std::string, std::vector< std::string > >::iterator src =
      _transitions.begin();
  for (; src != _transitions.end(); src++) {
    std::vector< std::string >::iterator tgt = src->second.begin();
    for (; tgt != src->second.end(); tgt++) {
      edges.push_back(std::make_pair(bblock_to_descriptor[src->first],
                                     bblock_to_descriptor[*tgt]));
    }
  }

  const int nedges = edges.size();
  std::vector< int > weights(nedges);
  fill(weights.begin(), weights.end(), 1);

  Graph g(&edges[0], &edges[0] + nedges, &weights[0], _bblocks.size());

  std::unordered_map< std::string, std::vector< std::string > >
      text_bblocks; // bblock name ID to a vertex;
  text_bblocks.reserve(_bblocks.size());

  std::unordered_map< std::string,
                      std::vector< boost::optional< s_expression > > >::iterator
      p = _bblocks.begin();
  for (; p != _bblocks.end(); p++) {
    std::vector< std::string > instructions;
    std::vector< boost::optional< s_expression > >::iterator i =
        p->second.begin();
    for (; i != p->second.end(); i++) {
      std::ostringstream ss;
      ss << **i;
      instructions.push_back(ss.str());
    }
    text_bblocks[p->first] = instructions;
  }

  write_graphviz(out, g, make_vertex_writer(&vertex_labels[0], text_bblocks));
}

/**
 * ARFunction implementation
 */
enum UnifiedExitBlockType { RETURN, UNREACHABLE, NONE };

ARFunction::ARFunction(Function* f,
                       BasicBlock* return_block,
                       BasicBlock* unreachable_block)
    : _next_llvm_inst(NULL) {
  _cfg.set_entry_block(f->getEntryBlock().getName().str());
  _name = f->getName().str();

  _func_type_id = refs->get_type_refid(f->getFunctionType());

  // Translating the function's formal parameters
  Function::arg_iterator arg_it = f->arg_begin();
  for (; arg_it != f->arg_end(); ++arg_it) {
    const Type* arg_type = arg_it->getType();
    std::string arg_name = arg_it->getName().str();

    s_expression_ostream fa("p");

    s_expression_ostream n("name");
    n << string_atom(arg_name);
    fa << n.expr();

    s_expression_ostream ty("ty");
    ty << index64_atom(refs->get_type_refid(arg_type));
    fa << ty.expr();

    _args.push_back(fa.expr());
  }

  // Translating instructions
  Function::iterator bb = f->begin();
  for (; bb != f->end(); bb++) {
    std::string name = bb->getName().str();

    // Check if this basic block is a return block.
    UnifiedExitBlockType exit_ty = NONE;
    if (return_block && (name == return_block->getName().str()))
      exit_ty = RETURN;
    else if (unreachable_block && (name == unreachable_block->getName().str()))
      exit_ty = UNREACHABLE;

    _cfg.enter_basic_block(name);
    BasicBlock::iterator i = bb->begin();
    for (; i != bb->end(); i++) {
      Instruction* inst = &*i;
      _next_llvm_inst = i->getNextNode();
      refs->set_debug_info(inst);
      _cfg.add_statement(translate_instruction(inst));
      refs->reset_debug_info();
    }

    if (exit_ty == RETURN)
      _cfg.set_exit_block(_cfg.get_current_block());
    if (exit_ty == UNREACHABLE)
      _cfg.set_unreachable_block(_cfg.get_current_block());
  }

#ifdef DEBUG
  std::cerr << "Function " << _name << ":" << std::endl;
  std::cerr << "  UnifyFunctionExitNodes.return: "
            << ((return_block == nullptr) ? "null"
                                          : return_block->getName().str())
            << std::endl;
  std::cerr << "  UnifyFunctionExitNodes.unreachable: "
            << ((unreachable_block == nullptr)
                    ? "null"
                    : unreachable_block->getName().str())
            << std::endl;
#endif
}

s_expression_ref ARFunction::gen_sexpr_intrinsic_call(ARCode& cfg,
                                                      CallInst* call) {
  // Check for intrinsics

  // Direct function call
  Function* called_function = call->getCalledFunction();
  std::string callee_name = called_function->getName().str();

  unsigned int id = called_function->getIntrinsicID();

  if (id == Intrinsic::dbg_declare || id == Intrinsic::dbg_value) {
    // This part of the code was originally implemented to
    // collect type  signedness. Now type signedness should
    // have been process in a separate pass that assigns
    // signedness to each register.

    // Return no instruction
    return s_expression_ref();
  }

  if (id == Intrinsic::memset || id == Intrinsic::memcpy ||
      id == Intrinsic::memmove) {
    std::string opname;
    std::string nameofarg1 = "src";
    if (id == Intrinsic::memset) {
      opname = "memset";
      nameofarg1 = "val";
    } else if (id == Intrinsic::memmove)
      opname = "memmove";
    else if (id == Intrinsic::memcpy)
      opname = "memcpy";

    s_expression_ostream meminst((string_atom(opname)));

    // Cast to MemIntrinsic
    MemIntrinsic* memint = dyn_cast< MemIntrinsic >(call);

    // Return value/type
    s_expression_ostream ret("ret");
    ret << refs->get_value(cfg, call);
    meminst << ret.expr();

    s_expression_ostream dest("dest");
    dest << refs->get_value(cfg, memint->getRawDest());
    meminst << dest.expr();

    s_expression_ostream arg1((string_atom(nameofarg1)));
    arg1 << refs->get_value(cfg, call->getArgOperand(1));
    meminst << arg1.expr();

    s_expression_ostream len("len");
    len << refs->get_value(cfg, memint->getLength());
    meminst << len.expr();

    s_expression_ostream align("align");
    align << z_number_atom(memint->getAlignment());
    meminst << align.expr();

    s_expression_ostream isvolatile("isvolatile");
    isvolatile << refs->get_value(cfg, memint->getVolatileCst());
    meminst << isvolatile.expr();

    meminst << refs->get_debug_sexpr();

    return meminst.expr();
  } else if (id == Intrinsic::vastart || id == Intrinsic::vaend ||
             id == Intrinsic::vacopy) {
    switch (id) {
      case Intrinsic::vastart: {
        s_expression_ostream vastart("va_start");
        vastart << refs->get_value(cfg, call->getArgOperand(0))
                << refs->get_debug_sexpr();
        return vastart.expr();
      }
      case Intrinsic::vaend: {
        s_expression_ostream vaend("va_end");
        vaend << refs->get_value(cfg, call->getArgOperand(0))
              << refs->get_debug_sexpr();
        return vaend.expr();
      }
      case Intrinsic::vacopy: {
        s_expression_ostream vacopy("va_copy"), dest("dest"), src("src");
        vacopy << (dest << refs->get_value(cfg, call->getArgOperand(0))).expr()
               << (src << refs->get_value(cfg, call->getArgOperand(1))).expr()
               << refs->get_debug_sexpr();
        return vacopy.expr();
      }
      default: {
        throw arbos_pass_error(
            "Unknown error while translating vararg operations");
      }
    }
  } else {
    // Translate other intrinsic calls to regular calls

    // Collect arguments/types and translate to s-expr
    std::vector< s_expression_ref > arg_operands;
    for (unsigned int i = 0; i < call->getNumArgOperands(); i++) {
      Value* arg = call->getArgOperand(i);
      arg_operands.push_back(refs->get_value(cfg, arg));
    }
    s_expression ret = refs->get_value(cfg, call);
    s_expression called_value = refs->get_value(cfg, call->getCalledValue());
    return gen_sexpr_call(ret, called_value, arg_operands, call->isInlineAsm());
  }

  return s_expression_ref();
}

Instruction* find_next_dbg(BasicBlock* entry) {
  std::deque< BasicBlock* > worklist;
  std::vector< BasicBlock* > done;

  // breadth-first search on successors
  worklist.push_back(entry);

  while (!worklist.empty()) {
    BasicBlock* bb = worklist.front();
    worklist.pop_front();

    for (auto it = bb->begin(); it != bb->end(); ++it) {
      if (it->getDebugLoc()) {
        return &*it;
      }
    }

    done.push_back(bb);
    for (auto succ : successors(bb)) {
      if (std::find(done.begin(), done.end(), succ) == done.end()) {
        worklist.push_back(succ);
      }
    }
  }

  return nullptr;
}

Instruction* find_pred_dbg(BasicBlock* entry) {
  std::deque< BasicBlock* > worklist;
  std::vector< BasicBlock* > done;

  // breadth-first search on predecessors
  worklist.push_back(entry);

  while (!worklist.empty()) {
    BasicBlock* bb = worklist.front();
    worklist.pop_front();

    for (auto it = bb->rbegin(); it != bb->rend(); ++it) {
      if (it->getDebugLoc()) {
        return &*it;
      }
    }

    done.push_back(bb);
    for (auto pred : predecessors(bb)) {
      if (std::find(done.begin(), done.end(), pred) == done.end()) {
        worklist.push_back(pred);
      }
    }
  }

  return nullptr;
}

s_expression_ref ARFunction::translate_instruction(Instruction* inst) {
  switch (inst->getOpcode()) {
    case Instruction::Call: { // call a function
      CallInst* call = dyn_cast< CallInst >(inst);
      Function* callee = call->getCalledFunction();

      if (callee && callee->isIntrinsic()) {
        return gen_sexpr_intrinsic_call(_cfg, call);
      }

      // General case: direct call to a function that is not intrinsic or
      // indirect call

      // Collect arguments/types and translate to s-expr
      std::vector< s_expression_ref > arg_operands;
      for (unsigned int i = 0; i < call->getNumArgOperands(); i++) {
        arg_operands.push_back(refs->get_value(_cfg, call->getArgOperand(i)));
      }
      return gen_sexpr_call(refs->get_value(_cfg, inst),
                            refs->get_value(_cfg, call->getCalledValue()),
                            arg_operands,
                            call->isInlineAsm());
    } break;

    case Instruction::Invoke: { // Method call
      InvokeInst* invoke = dyn_cast< InvokeInst >(inst);
      s_expression_ostream op("invoke");

      // Generate call
      // Collect arguments/types and translate to s-expr
      std::vector< s_expression_ref > arg_operands;
      for (unsigned int i = 0; i < invoke->getNumArgOperands(); i++) {
        arg_operands.push_back(refs->get_value(_cfg, invoke->getArgOperand(i)));
      }
      op << gen_sexpr_call(refs->get_value(_cfg, inst),
                           refs->get_value(_cfg, invoke->getCalledValue()),
                           arg_operands,
                           false);

      // Generate branches
      s_expression_ostream br("br"), normal("normal"), except("exception");
      std::string normal_bb = invoke->getNormalDest()->getName().str();
      std::string except_bb = invoke->getUnwindDest()->getName().str();

      std::string current_ar_bblock = _cfg.get_current_block();
      _cfg.add_transition(current_ar_bblock, normal_bb);
      _cfg.add_transition(current_ar_bblock, except_bb);

      br << (normal << string_atom(normal_bb)).expr()
         << (except << string_atom(except_bb)).expr();
      op << br.expr();

      op << refs->get_debug_sexpr();
      return op.expr();
    } break;

    case Instruction::Select: {
      SelectInst* select = dyn_cast< SelectInst >(inst);
      refs->set_debug_info(select);
      _cfg.split_blocks(select, select->getCondition());
      refs->reset_debug_info();
    } break;

    case Instruction::ICmp:   // Integer comparison instruction
    case Instruction::FCmp: { // Floating point comparison instr.
      // Identify if this CompInst is a boolean operation
      assert(_next_llvm_inst);
      BranchInst* br = dyn_cast< BranchInst >(_next_llvm_inst);
      if (br == 0 || br->isUnconditional()) {
        CmpInst* cmp = dyn_cast< CmpInst >(inst);
        _cfg.split_blocks(cmp, cmp);
      }
    } break;

    case Instruction::ExtractValue: { // extract from aggregate
      return refs->translate_extractvalue(_cfg,
                                          dyn_cast< ExtractValueInst >(inst));
    } break;

    case Instruction::InsertValue: { // insert into aggregate
      return refs->translate_insertvalue(_cfg,
                                         dyn_cast< InsertValueInst >(inst));
    } break;

    case Instruction::ExtractElement: { // extract from vector
      return refs->translate_extractelement(_cfg,
                                            dyn_cast< ExtractElementInst >(
                                                inst));
    } break;

    case Instruction::InsertElement: { // insert into vector
      return refs->translate_insertelement(_cfg,
                                           dyn_cast< InsertElementInst >(inst));
    } break;

    // Terminator Instructions - These instructions are used to terminate a
    // basic
    // block of the program.   Every basic block must end with one of these
    // instructions for it to be a well formed basic block.
    case Instruction::Br: {
      BranchInst* br = dyn_cast< BranchInst >(inst);

      if (br->isUnconditional()) {
        std::string next = br->getSuccessor(0)->getName().str();
        _cfg.route_to_new_dest(br->getParent()->getName().str(), next);
      } else {
        std::string orig_br_true = br->getSuccessor(0)->getName().str();
        std::string orig_br_false = br->getSuccessor(1)->getName().str();
        std::string current_llvm_bblock = br->getParent()->getName().str();

        std::string current_ar_bblock = _cfg.get_current_block();

        Value* condition = br->getCondition();
        CmpInst* cmp = dyn_cast< CmpInst >(condition);
        std::string opcodename = "bool"; // default boolean op
        if (Instruction* cond_inst =
                dyn_cast< Instruction >(br->getCondition())) {
          opcodename = Instruction::getOpcodeName(cond_inst->getOpcode());
        }

        std::string br_true = "*out_" + current_llvm_bblock + "_to_" +
                              orig_br_true + "_" + opcodename + "_true";
        _cfg.enter_basic_block(br_true);

        // Create connections for current_ar_bblock -> br_true -> the true
        // destination
        // (either the original or the new one if available)
        _cfg.add_transition(current_ar_bblock, br_true);
        std::string new_br_true =
            _cfg.get_new_destination(current_llvm_bblock, orig_br_true);
        _cfg.add_transition(br_true, new_br_true);

        // Remove transitions that by passes br_true
        _cfg.remove_transition(current_ar_bblock, orig_br_true);
        _cfg.remove_transition(current_ar_bblock, new_br_true);

        Instruction *bb_true_dbg = NULL, *bb_false_dbg = NULL;
        if (cmp && cmp->getDebugLoc()) {
          bb_true_dbg = bb_false_dbg = cmp;
        } else if (!br->getDebugLoc()) {
          bb_true_dbg = find_next_dbg(br->getSuccessor(0));
          bb_false_dbg = find_next_dbg(br->getSuccessor(1));
        }

        if (bb_true_dbg) {
          refs->set_debug_info(bb_true_dbg);
        }

        if (cmp) {
          _cfg.add_statement(gen_assertion(_cfg, cmp, true));
          _cfg.add_statement(
              gen_assign(_cfg, cmp, ConstantInt::getTrue(cmp->getType())));
        } else {
          _cfg.add_statement(gen_assertion(_cfg, condition, true));
        }

        std::string br_false = "*out_" + current_llvm_bblock + "_to_" +
                               orig_br_false + "_" + opcodename + "_false";
        _cfg.enter_basic_block(br_false);

        // Create connections for current_ar_bblock -> br_false -> the false
        // destination
        // (either the original or the new one if available)
        _cfg.add_transition(current_ar_bblock, br_false);
        std::string new_br_false =
            _cfg.get_new_destination(current_llvm_bblock, orig_br_false);
        _cfg.add_transition(br_false, new_br_false);

        // Remove transitions that by passes br_false
        _cfg.remove_transition(current_ar_bblock, orig_br_false);
        _cfg.remove_transition(current_ar_bblock, new_br_false);

        if (bb_false_dbg) {
          refs->set_debug_info(bb_false_dbg);
        }

        if (cmp) {
          _cfg.add_statement(gen_assertion(_cfg, cmp, false));
          _cfg.add_statement(
              gen_assign(_cfg, cmp, ConstantInt::getFalse(cmp->getType())));
        } else {
          _cfg.add_statement(gen_assertion(_cfg, condition, false));
        }

        if (bb_true_dbg || bb_false_dbg) {
          refs->reset_debug_info();
        }
      }
    } break;

    case Instruction::PHI: { // PHI node instruction
      PHINode* phi = dyn_cast< PHINode >(inst);
      std::string current_llvm_bblock = phi->getParent()->getName().str();

      for (unsigned int i = 0; i < phi->getNumIncomingValues(); ++i) {
        std::string current_ar_bblock = _cfg.get_current_block();
        Value* incoming_value = phi->getIncomingValue(i);
        std::string incoming_block_name =
            phi->getIncomingBlock(i)->getName().str();

        // Get source location through incoming branch instruction
        TerminatorInst* term = phi->getIncomingBlock(i)->getTerminator();
        assert(term && "incoming basic block to phi is missing a terminator");

        if (term->getDebugLoc()) {
          refs->set_debug_info(term);
        } else {
          Instruction* dbg_inst = find_pred_dbg(term->getParent());
          if (dbg_inst) {
            refs->set_debug_info(dbg_inst);
          } else {
            refs->set_debug_info(term);
          }
        }

        s_expression s_assign = gen_assign(_cfg, inst, incoming_value);
        refs->reset_debug_info();

        std::string opcodename = Instruction::getOpcodeName(phi->getOpcode());
        std::string phi_assign_bblock = "*in_" + incoming_block_name + "_to_" +
                                        current_llvm_bblock + "_" + opcodename;
        _cfg.enter_basic_block(phi_assign_bblock);
        _cfg.add_statement(s_assign);
        _cfg.add_transition(phi_assign_bblock, current_llvm_bblock);

        // Remove transitions to current_llvm_bblock, which contains the phi
        // instruction
        _cfg.remove_transition(incoming_block_name, current_llvm_bblock);
        std::string new_incoming_block_name =
            _cfg.get_new_source(incoming_block_name, current_llvm_bblock);
        _cfg.remove_transition(new_incoming_block_name, current_llvm_bblock);

        _cfg.add_transition(new_incoming_block_name, phi_assign_bblock);
      }
      _cfg.enter_basic_block(current_llvm_bblock);
    } break;

    case Instruction::Alloca: { // Stack management
      AllocaInst* alloca = dyn_cast< AllocaInst >(inst);
      s_expression_ostream local_var("local_var"), a("allocate"),
          alloca_ty("alloca_ty"), array_size("array_size"), dest("dest");

      // Create s-expression for the local variable
      local_var << create_var_sexpr(alloca->getName().str(),
                                    refs->get_type_refid(alloca->getType()),
                                    alloca->getAlignment());
      _local_vars.push_back(local_var.expr());

      // Create the allocate instruction associated with the local variable
      a << (dest << refs->translate_constant_local_addr(_cfg, alloca)).expr()
        << (alloca_ty << index64_atom(
                refs->get_type_refid(alloca->getAllocatedType())))
               .expr()
        << (array_size << refs->get_value(_cfg, alloca->getArraySize())).expr()
        << refs->get_debug_sexpr();

      return a.expr();
    } break;

    case Instruction::IndirectBr:    // Goto with label as value
    case Instruction::ShuffleVector: // shuffle two vectors.
    case Instruction::UserOp1:       // May be used internally in a pass
    case Instruction::UserOp2: {     // Internal to passes only
      std::string opcodename = Instruction::getOpcodeName(inst->getOpcode());
      std::ostringstream buf;
      buf << "Unsupported construct: operation '" << opcodename << "'";
      throw arbos_pass_error(buf.str());
    } break;

    case Instruction::Switch: { // Switch statements are always lowered
      throw arbos_pass_error(
          "Incorrect pass configuration: 'switch' statements must be lowered "
          "first");
    } break;

    case Instruction::VAArg: { // va_arg instruction
      VAArgInst* vararg = dyn_cast< VAArgInst >(inst);

      s_expression_ostream op("va_arg"), ret("ret"), ptr("ptr");
      op << (ret << refs->get_value(_cfg, vararg)).expr()
         << (ptr << refs->get_value(_cfg, vararg->getPointerOperand())).expr()
         << create_ty_ref_sexpr(refs->get_type_refid(vararg->getType()))
         << refs->get_debug_sexpr();
      return op.expr();
    } break;

    case Instruction::Ret: { // Return statement
      s_expression_ostream op("ret");
      s_expression_ostream rhs("rhs");
      if (inst->getNumOperands() == 1) {
        rhs << refs->get_value(_cfg, inst->getOperand(0));
      }
      op << rhs.expr();
      op << refs->get_debug_sexpr();
      return op.expr();
    } break;

    case Instruction::Unreachable: { // Marker for unreachable control point
      s_expression_ostream op("unreachable");
      op << refs->get_debug_sexpr();
      return op.expr();
    } break;

    case Instruction::LandingPad: {
      s_expression_ostream op("landingpad");
      s_expression_ostream exception("exception");
      exception << refs->get_value(_cfg, inst);
      op << exception.expr() << refs->get_debug_sexpr();
      return op.expr();
    } break;

    case Instruction::Resume: {
      _cfg.set_ehresume_block(_cfg.get_current_block());
      s_expression_ostream op("resume");
      s_expression_ostream exception("exception");
      exception << refs->get_value(_cfg, inst);
      op << exception.expr() << refs->get_debug_sexpr();
      return op.expr();
    } break;

    case Instruction::Store: {
      StoreInst* storeinst = dyn_cast< StoreInst >(inst);
      s_expression_ostream op("store");
      s_expression_ostream lhs("lhs");
      lhs << refs->get_value(_cfg, inst->getOperand(1));
      s_expression_ostream rhs("rhs");
      rhs << refs->get_value(_cfg, inst->getOperand(0));
      op << lhs.expr() << rhs.expr()
         << create_align_sexpr(storeinst->getAlignment())
         << refs->get_debug_sexpr();
      return op.expr();
    } break;

    case Instruction::GetElementPtr: {
      return refs->translate_getelementptr(_cfg, inst);
    } break;

    // Standard binary operators...
    case Instruction::Add:
    case Instruction::FAdd:
    case Instruction::Sub:
    case Instruction::FSub:
    case Instruction::Mul:
    case Instruction::FMul:
    case Instruction::UDiv:
    case Instruction::SDiv:
    case Instruction::FDiv:
    case Instruction::URem:
    case Instruction::SRem:
    case Instruction::FRem:

    // Logical operators (integer operands)
    case Instruction::Shl:  // Shift left  (logical)
    case Instruction::LShr: // Shift right (logical)
    case Instruction::AShr: // Shift right (arithmetic)
    case Instruction::And:
    case Instruction::Or:
    case Instruction::Xor:

    // Memory operators...
    case Instruction::Load: // Memory manipulation instrs

    // Cast operators ...
    case Instruction::Trunc:    // Truncate integers
    case Instruction::ZExt:     // Zero extend integers
    case Instruction::SExt:     // Sign extend integers
    case Instruction::FPToUI:   // floating point -> UInt
    case Instruction::FPToSI:   // floating point -> SInt
    case Instruction::UIToFP:   // UInt -> floating point
    case Instruction::SIToFP:   // SInt -> floating point
    case Instruction::FPTrunc:  // Truncate floating point
    case Instruction::FPExt:    // Extend floating point
    case Instruction::PtrToInt: // Pointer -> Integer
    case Instruction::IntToPtr: // Integer -> Pointer
    case Instruction::BitCast:  // Type cast

    default: {
      std::string opcodename = Instruction::getOpcodeName(inst->getOpcode());

      s_expression_ostream op((string_atom(opcodename)));

      s_expression_ostream lhs("lhs");
      if (!inst->getType()->isVoidTy()) {
        lhs << refs->get_value(_cfg, inst);
      }
      op << lhs.expr();

      s_expression_ostream rhs("rhs");
      for (unsigned int i = 0; i < inst->getNumOperands(); i++) {
        rhs << refs->get_value(_cfg, inst->getOperand(i));
      }
      op << rhs.expr();

      if (inst->getOpcode() == Instruction::Load) {
        LoadInst* loadinst = dyn_cast< LoadInst >(inst);
        op << create_align_sexpr(loadinst->getAlignment());
      }

      if (!inst->getDebugLoc()) {
        Instruction* next_inst = inst->getNextNode();
        if (next_inst && next_inst->getDebugLoc()) {
          refs->set_debug_info(next_inst);
        }
      }

      op << refs->get_debug_sexpr();

      if (!inst->getDebugLoc()) {
        refs->reset_debug_info();
      }
      return op.expr();
    } break;
  } // end switch

  return s_expression_ref();
}

void ARFunction::printAR(std::ostream& outf) {
  s_expression_ostream s("function");

  s_expression_ostream n("name");
  n << string_atom(_name);
  s << n.expr();

  s_expression_ostream f_ty("ty");
  f_ty << index64_atom(_func_type_id);
  s << f_ty.expr();

  s_expression_ostream s_args("params");
  std::vector< boost::optional< s_expression > >::iterator p = _args.begin();
  for (; p != _args.end(); p++) {
    s_args << **p;
  }
  s << s_args.expr();

  s_expression_ostream s_localvars("local_vars");
  p = _local_vars.begin();
  for (; p != _local_vars.end(); p++) {
    s_localvars << **p;
  }
  s << s_localvars.expr();

  s << _cfg.expr();

  s_expression_formatter fmt(outf);
  fmt << sexpr_indent("function");
  fmt << sexpr_indent("localvars");
  fmt << sexpr_indent("code");
  fmt << sexpr_indent("basicblocks");
  fmt << sexpr_indent("basicblock");
  fmt << sexpr_indent("instructions");
  fmt << sexpr_indent("trans");
  fmt << s.expr();

  outf << std::endl;
}

void ARFunction::printDOT() {
  std::ofstream outf;
  std::string fname = _name + ".dot";
  outf.open(fname.c_str());
  _cfg.printDOT(outf);
  outf.close();
}
