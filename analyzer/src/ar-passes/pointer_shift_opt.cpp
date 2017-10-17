/*******************************************************************************
 *
 * Remove redundant arithmetic statements produced by the translation
 * of LLVM getElementPtr instructions to ARBOS pointer shifts.
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

#include <algorithm>
#include <iterator>

#include <ikos/domains/discrete_domains.hpp>

#include <analyzer/analysis/common.hpp>
#include <analyzer/ar-wrapper/cfg.hpp>
#include <analyzer/ar-wrapper/constant_folding.hpp>
#include <analyzer/ar-wrapper/literal.hpp>
#include <analyzer/ar-wrapper/transformations.hpp>
#include <analyzer/ar-wrapper/wrapper.hpp>

namespace arbos {
namespace passes {

using namespace arbos::transformations;
using namespace analyzer;

class PointerShiftOptPass : public Pass {
  typedef std::vector< Statement_ref >::reverse_iterator stmt_rev_iterator;

  // Simple backward data slicing starting from target but only at the
  // level of basic-block.
  template < typename ReverseIterator >
  std::vector< Statement_ref > Slice(ReverseIterator begin,
                                     ReverseIterator end,
                                     varname_set_t target,
                                     live_info::live_info_ptr live) {
    std::vector< Statement_ref > slice;
    for (ReverseIterator it = begin; it != end; ++it) {
      varname_set_t defined = live->defs(*it);
      if (!(defined & target).empty()) {
        slice.insert(slice.begin(), *it);
        target = target | live->uses(*it);
      }
    }
    return slice;
  }

  // Remove arithmetic operations with constant operands that feed
  // pointer shift statements.
  // For instance, the sequence of statements:
  //    o1 = 0; o2 = o1 + 8; x = pointer_shift (&a, o2);
  // can be replaced with:
  //    x = pointer_shift (&a, 8);
  // This simplification reduces significantly the number of variables
  // per block which has a big impact on numerical domains.
  void removeArithOps(Basic_Block_ref block,
                      VariableFactory& vfac,
                      LiteralFactory& lfac,
                      live_info::live_info_ptr live) {
    std::vector< Statement_ref >
        prev_stmts; // must be a container with reverse iterators
    StmtRange stmts = ar::getStatements(block);
    for (StmtRange::iterator I = stmts.begin(), E = stmts.end(); I != E; ++I) {
      Statement_ref S = *I;
      if (ar::is_pointer_shift_stmt(S)) {
        Pointer_Shift_ref ps = node_cast< Pointer_Shift >(S);
        Operand_ref offset = ar::getOffset(ps);
        Operand_ref base = ar::getOffset(ps);
        scalar_lit_t offset_lit = lfac[offset];
        scalar_lit_t base_lit = lfac[base];
        varname_set_t target;

        if (offset_lit.is_integer_var()) {
          target += offset_lit.var();
        }
        if (base_lit.is_pointer_var()) {
          target += base_lit.var();
        }
        if (!target.empty()) {
          std::vector< Statement_ref > offset_stmts =
              Slice(prev_stmts.rbegin(), prev_stmts.rend(), target, live);
          std::vector< ConstantFolding::binding_t > new_offset_stmts;
          ConstantFolding simp;
          simp.apply(offset_stmts, new_offset_stmts);
          std::vector< Statement_ref >::iterator I1 = offset_stmts.begin();
          std::vector< ConstantFolding::binding_t >::iterator I2 =
              new_offset_stmts.begin();
          for (; I1 != offset_stmts.end(); ++I1, ++I2) {
            if ((*I2).second) { // *I1 is dead code
              removeStatement(block, (*I2).first);
            } else {
              replaceStatement(block, *I1, (*I2).first);
            }
          }
          Operand_ref new_offset = simp.remap(offset);
          setOffset(ps, new_offset);
          replaceStatement(block, S, ps);
        }
        prev_stmts.clear();
      } // end is_pointer_shift
      else {
        prev_stmts.push_back(S);
      }
    }
  }

  // Remove pointer shift statements. Currently it searches only for
  // this particular idiom which is very common:
  //    x = pointer_shift (&y, k1);
  //    x = pointer_shift (x, k2);
  // which is rewritten to
  //    x = pointer_shift (&y, k1+k2);
  // This simplification improves precision of flow-insensitive
  // pointer analyses.
  void removePointerShifts(Basic_Block_ref block) {
    typedef stmt_replace_operands< Operand_ref > stmt_replace_t;
    typedef stmt_replace_t::substitution_t subst_t;

    std::vector< Statement_ref > ps_s;
    StmtRange stmts = ar::getStatements(block);
    for (StmtRange::iterator I = stmts.begin(),
                             P = stmts.begin(),
                             E = stmts.end();
         I != E;) {
      if (ar::is_pointer_shift_stmt(*I)) {
        if (I == P) { // first time a pointer shift is seen
          ++I;
          continue;
        } else if (I == P + 1) { // second consecutive pointer shift
          Pointer_Shift_ref p = node_cast< Pointer_Shift >(*P);
          Pointer_Shift_ref c = node_cast< Pointer_Shift >(*I);
          // Operand_ref's cannot be compared directly using their
          // UID's. Instead, we need to use Operand_Hasher class
          arbos::Operand_Hasher::hash op_h;
          arbos::Internal_Variable_Hasher::hash iv_h;
          if (iv_h(ar::getResult(p)) == iv_h(ar::getResult(c)) &&
              iv_h(ar::getResult(c)) == op_h(ar::getBase(c))) {
            boost::optional< z_number > o1 = getIntCst(ar::getOffset(p));
            boost::optional< z_number > o2 = getIntCst(ar::getOffset(c));
            if (o1 && (*o1 == 0)) {
              stmt_replace_t::stmt_replace_ptr visitor(new stmt_replace_t(
                  subst_t(ar::getOffset(p), ar::getOffset(c))));
              ar::accept(*P, visitor);
              // --- For some reason to do this directly causes double-free:
              // setOffset(p, ar::getOffset(c));
              ps_s.push_back(*I);
            } else if (o1 && o2) {
              Operand_ref new_o =
                  makeIntCst(*o1 + *o2, ar::getType(ar::getOffset(c)));
              stmt_replace_t::stmt_replace_ptr visitor(
                  new stmt_replace_t(subst_t(ar::getOffset(p), new_o)));
              ar::accept(*P, visitor);
              // --- For some reason to do this directly causes double-free:
              // setOffset(p, new_o);
              ps_s.push_back(*I);
            }
          }
        }
      }
      // we just equate I and P
      ++I;
      P = I;
    } // end for

    // remove the redundant pointer shift statements
    for (std::vector< Statement_ref >::iterator it = ps_s.begin(),
                                                et = ps_s.end();
         it != et;
         ++it) {
      removeStatement(block, *it);
    }
  }

  // void removePointerShifts(Basic_Block_ref block) {
  //   std::vector< Statement_ref > ps_stmts;
  //   StmtRange stmts = ar::getStatements(block);
  //   for (StmtRange::iterator I = stmts.begin(), E = stmts.end(); I != E; ++I)
  //   {
  //     if (ar::is_pointer_shift_stmt(*I))
  //       ps_stmts.push_back(*I);
  //   }
  //   std::vector< ConstantFolding::binding_t > new_ps_stmts;
  //   ConstantFolding simp;
  //   simp.apply(ps_stmts, new_ps_stmts);
  //   std::vector< Statement_ref >::iterator I1 = ps_stmts.begin();
  //   std::vector< ConstantFolding::binding_t >::iterator I2 =
  //   new_ps_stmts.begin();
  //   for (; I1 != ps_stmts.end(); ++I1, ++I2) {
  //     if ((*I2).second) // *I1 is dead code
  //     {
  //       removeStatement(block, (*I2).first);
  //     } else {
  //       replaceStatement(block, *I1, (*I2).first);
  //     }
  //   }
  // }

public:
  PointerShiftOptPass() : Pass("ps-opt", "Optimize pointer shift statements") {}

  void execute(Bundle_ref bundle) {
    try {
      VariableFactory vfac;
      LiteralFactory lfac(vfac);

      FuncRange entries = ar::getFunctions(bundle);
      for (FuncRange::iterator fI = entries.begin(), fE = entries.end();
           fI != fE;
           ++fI) {
        BBRange blks = ar::getBlocks(*fI);
        for (BBRange::iterator bI = blks.begin(), bE = blks.end(); bI != bE;
             ++bI) {
          Basic_Block_ref b = *bI;
          live_info::live_info_ptr live(new live_info(vfac, PTR));
          ar::accept(b, live);

          removeArithOps(b, vfac, lfac, live);
          removePointerShifts(b);
        }
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
};

} // end namespace passes
} // end namespace arbos

extern "C" arbos::Pass* init() {
  return new arbos::passes::PointerShiftOptPass();
}
