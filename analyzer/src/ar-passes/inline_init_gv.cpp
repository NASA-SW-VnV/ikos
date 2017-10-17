/*******************************************************************************
 *
 * Create a function __ikos_init_globals() containing the initialization
 * of global variables and add a call to __ikos_init_globals() in main().
 *
 * Authors: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2017 United States Government as represented by the
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

#include <algorithm>
#include <iterator>

#include <analyzer/analysis/common.hpp>
#include <analyzer/ar-wrapper/cfg.hpp>
#include <analyzer/ar-wrapper/transformations.hpp>
#include <analyzer/ar-wrapper/wrapper.hpp>

namespace arbos {

namespace passes {

using namespace arbos::transformations;
using namespace analyzer;

static Option< std::vector< std::string > > INIT_GLOBALS(
    "init-globals",
    "List of functions where global variables should be initialized "
    "(default: main)");
static Option< bool > ONLY_SCALARS("only-scalars",
                                   "Initialize only scalar global variables");
static Option< bool > ONLY_POINTERS("only-pointers",
                                    "Initialize only pointer global variables");

class InlineInitGV : public Pass {
public:
  InlineInitGV()
      : Pass("inline-init-gv",
             "Inline initialization of global variables in main") {}

  void execute(Bundle_ref bundle) {
    try {
      const char* init_gv_name = "__ikos_init_globals";

      // check if __ikos_init_globals() already exists
      if (bundle->getFunctionByNameId(init_gv_name)) {
        std::ostringstream buf;
        buf << "function " << init_gv_name << " already exists";
        throw analyzer_error(buf.str());
      }

      // create a void type
      Void_Type_ref void_type = Void_Type::create();

      // create function __ikos_init_globals()
      Function_Type_ref init_gv_type =
          Function_Type::create(void_type, std::vector< Type_ref >{}, false);
      Function_ref init_gv_fun =
          Function::create(init_gv_name,
                           init_gv_type,
                           std::vector< Internal_Variable_ref >{});
      Code_ref init_gv_code = init_gv_fun->getFunctionBody();
      Basic_Block_ref init_gv_bb = Basic_Block::create("entry");

      bundle->addFunction(init_gv_fun);
      init_gv_code->addBasicBlock(init_gv_bb);
      init_gv_code->setEntryBlock(init_gv_bb);
      init_gv_code->setExitBlock(init_gv_bb);

      // merge all initialization code into __ikos_init_globals()
      GvRange globals = ar::getGlobalVars(bundle);
      for (GvRange::iterator it = globals.begin(), et = globals.end(); it != et;
           ++it) {
        Type_ref type = ar::getPointeeType(ar::getType(*it));
        bool is_scalar = ar::isInteger(type) || ar::isFloat(type);
        bool is_pointer =
            ar::isPointer(type) ||
            (ar::isArray(type) && ar::isPointer(ar::getCellType(type)));

        if ((ONLY_SCALARS && ONLY_POINTERS && !is_scalar && !is_pointer) ||
            (ONLY_SCALARS && !ONLY_POINTERS && !is_scalar) ||
            (!ONLY_SCALARS && ONLY_POINTERS && !is_pointer)) {
          continue;
        }

        StmtRange stmts = getInitialization(*it);
        for (StmtRange::iterator it = stmts.begin(), et = stmts.end(); it != et;
             ++it) {
          init_gv_bb->pushBack(*it);
        }
      }

      // for c++, check the global variable llvm.global_ctors
      GvRange::iterator it =
          std::find_if(globals.begin(),
                       globals.end(),
                       [](const Global_Variable_ref& g) {
                         return g->getNameId() == "llvm.global_ctors";
                       });
      if (it != globals.end()) {
        int next_id = 0;
        StmtRange stmts = getInitialization(*it);

        // find store of function pointers
        for (StmtRange::iterator it = stmts.begin(), et = stmts.end(); it != et;
             ++it) {
          Statement_ref stmt = *it;

          if (ar::is_store_stmt(stmt)) {
            Store_ref store = node_cast< Store >(stmt);
            Operand_ref op = ar::getValue(store);
            if (ar::isFunctionPointer(op)) {
              Internal_Variable_ref dummy_var =
                  Internal_Variable::create(void_type,
                                            "__v:init-gv-" +
                                                std::to_string(next_id++));
              init_gv_code->addInternalVariable(dummy_var);
              Statement_ref call_stmt =
                  Call::create(dummy_var,
                               op,
                               std::vector< Operand_ref >{},
                               false,
                               AR_Null_Ref);
              init_gv_bb->pushBack(call_stmt);
            }
          }
        }
      }

      // add return statement
      Return_Value_ref return_stmt = Return_Value::create(AR_Null_Ref);
      init_gv_bb->pushBack(return_stmt);

      // collect functions where we should add a call to __ikos_init_globals()
      std::vector< Function_ref > functions;
      if (INIT_GLOBALS) {
        const std::vector< std::string >& init_globals =
            INIT_GLOBALS.getValue();

        for (auto it = init_globals.begin(); it != init_globals.end(); ++it) {
          boost::optional< Function_ref > fun = ar::getFunction(bundle, *it);

          if (!fun) {
            std::ostringstream buf;
            buf << "could not find function " << *it;
            throw analyzer_error(buf.str());
          }

          functions.push_back(*fun);
        }
      } else {
        boost::optional< Function_ref > main_fun =
            ar::getFunction(bundle, "main");

        if (!main_fun) {
          throw analyzer_error("could not find main()");
        }

        functions.push_back(*main_fun);
      }

      // add calls to __ikos_init_globals()
      for (auto it = functions.begin(); it != functions.end(); ++it) {
        Basic_Block_ref fun_entry_bb = ar::getEntryBlock(*it);

        // guess a source location for the call
        Source_Location_ref src_loc = first_source_location(ar::getBody(*it));

        // create and add call statement
        Internal_Variable_ref dummy_var =
            Internal_Variable::create(void_type, "__v:init-gv-0");
        (*it)->getFunctionBody()->addInternalVariable(dummy_var);
        Operand_ref init_gv_op = Cst_Operand::create(
            Function_Addr_Constant::create(init_gv_type, init_gv_name));
        Statement_ref call_stmt = Call::create(dummy_var,
                                               init_gv_op,
                                               std::vector< Operand_ref >{},
                                               false,
                                               src_loc);
        fun_entry_bb->insertFront(call_stmt);
      }
    } catch (analyzer_error& e) {
      std::cerr << "analyzer error: " << e << std::endl;
      exit(EXIT_FAILURE);
    } catch (ikos::exception& e) {
      std::cerr << "ikos error: " << e.what() << std::endl;
      exit(EXIT_FAILURE);
    } catch (arbos::error& e) {
      std::cerr << "arbos error: " << e << std::endl;
      exit(EXIT_FAILURE);
    } catch (std::exception& e) {
      std::cerr << "system error: " << e.what() << std::endl;
      exit(EXIT_FAILURE);
    } catch (...) {
      std::cerr << "unknown error occurred" << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  static Source_Location_ref first_source_location(Code_ref code) {
    std::deque< Basic_Block_ref > worklist;
    std::vector< Basic_Block_ref > done;

    // breadth-first search from the entry block
    worklist.push_back(code->getEntryBlock());

    while (!worklist.empty()) {
      Basic_Block_ref bb = worklist.front();
      worklist.pop_front();

      StmtRange stmts = bb->getStatements();
      for (const Statement_ref& stmt : stmts) {
        if (stmt->getSourceLocation() != Null_ref &&
            stmt->getSourceLocation()->getLineNumber() != -1 &&
            stmt->getSourceLocation()->getColumnNumber() != -1) {
          return stmt->getSourceLocation();
        }
      }

      // could not find any source location, look further
      done.push_back(bb);
      auto succs = bb->getNextBlocks();
      for (auto it = succs.first; it != succs.second; ++it) {
        if (std::find(done.begin(), done.end(), *it) == done.end()) {
          worklist.push_back(*it);
        }
      }
    }

    return Null_ref;
  }

}; // end class InlineInitGV

} // end namespace passes
} // end namespace arbos

extern "C" arbos::Pass* init() {
  return new arbos::passes::InlineInitGV();
}
