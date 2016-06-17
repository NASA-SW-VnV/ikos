/*******************************************************************************
 *
 * Inline initialization of global variables to the main function.
 *
 * Authors: Jorge A. Navas
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
    FuncRange functions = ar::getFunctions(bundle);
    FuncRange::iterator it =
        find_if(functions.begin(), functions.end(), IsMain);

    if (it == functions.end())
      return;

    Function_ref Main = *it;
    GvRange GVs = ar::getGlobalVars(bundle);
    boost::optional< Basic_Block_ref > entryBB = ar::getEntryBlock(Main);
    assert(entryBB && "Entry block not found");

    for (GvRange::iterator I = GVs.begin(), E = GVs.end(); I != E; ++I) {
      Type_ref type = ar::getPointeeType(ar::getType(*I));
      bool is_scalar = ar::isInteger(type) || ar::isFloat(type);
      bool is_pointer =
          ar::isPointer(type) ||
          (ar::isArray(type) && ar::isPointer(ar::getCellType(type)));

      if ((ONLY_SCALARS && ONLY_POINTERS && !is_scalar && !is_pointer) ||
          (ONLY_SCALARS && !ONLY_POINTERS && !is_scalar) ||
          (!ONLY_SCALARS && ONLY_POINTERS && !is_pointer)) {
        continue;
      }

      // We assume that StmtRange is just a ForwardIterator so we need
      // to put all elements into a BidirectionalIterator so then we
      // can traverse it in reverse order.
      StmtRange stmts = getInitialization(*I);
      typedef std::vector< Statement_ref > BiRange;
      BiRange r;
      for (StmtRange::iterator SI = stmts.begin(), SE = stmts.end(); SI != SE;
           ++SI) {
        r.push_back(*SI);
      }

      for (BiRange::reverse_iterator I = r.rbegin(), E = r.rend(); I != E;
           ++I) {
        addFrontStatement(*entryBB, *I);
      }
    }
  }

}; // end class InlineInitGV

} // end namespace passes
} // end namespace arbos

extern "C" arbos::Pass* init() {
  return new arbos::passes::InlineInitGV();
}
