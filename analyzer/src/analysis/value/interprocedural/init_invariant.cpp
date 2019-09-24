/*******************************************************************************
 *
 * \file
 * \brief Initial invariant for the interprocedural value analysis
 *
 * Author: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2019 United States Government as represented by the
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

#include <ikos/analyzer/analysis/literal.hpp>
#include <ikos/analyzer/analysis/value/interprocedural/init_invariant.hpp>
#include <ikos/analyzer/util/log.hpp>

namespace ikos {
namespace analyzer {
namespace value {
namespace interprocedural {

AbstractDomain init_main_invariant(Context& ctx,
                                   ar::Function* main,
                                   AbstractDomain inv) {
  const ScalarLit& argc = ctx.lit_factory->get_scalar(main->param(0));
  const ScalarLit& argv = ctx.lit_factory->get_scalar(main->param(1));

  if (!argc.is_machine_int_var()) {
    log::warning("unexpected type for first argument of main");
    return inv;
  }
  if (!argv.is_pointer_var()) {
    log::warning("unexpected type for second argument of main");
    return inv;
  }

  // Set argc
  auto argc_type = cast< ar::IntegerType >(main->param(0)->type());
  if (ctx.opts.argc) {
    // Add `argc = ctx.opts.argc`
    inv.normal().int_assign(argc.var(),
                            MachineInt(*ctx.opts.argc,
                                       argc_type->bit_width(),
                                       argc_type->sign()));
  } else {
    // Add `argc >= 0`
    inv.normal().int_assign_nondet(argc.var());
    inv.normal().int_add(core::machine_int::Predicate::GE,
                         argc.var(),
                         MachineInt::zero(argc_type->bit_width(),
                                          argc_type->sign()));
  }

  // Set argv
  ArgvMemoryLocation* argv_mem_loc = ctx.mem_factory->get_argv();
  inv.normal().pointer_assign(argv.var(),
                              argv_mem_loc,
                              core::Nullity::non_null());

  if (ctx.opts.argc) {
    // Add size of argv array
    const ar::DataLayout& dl = ctx.bundle->data_layout();
    uint64_t pointer_size = dl.pointers.bit_width / 8;
    uint64_t argv_size =
        pointer_size * (static_cast< uint64_t >(*ctx.opts.argc) + 1U);
    Variable* alloc_size_var = ctx.var_factory->get_alloc_size(argv_mem_loc);
    inv.normal().int_assign(alloc_size_var,
                            MachineInt(argv_size,
                                       dl.pointers.bit_width,
                                       Unsigned));

    // TODO(marthaud): Create memory locations for argv[i] and set the size >= 1
  }

  return inv;
}

} // end namespace interprocedural
} // end namespace value
} // end namespace analyzer
} // end namespace ikos
