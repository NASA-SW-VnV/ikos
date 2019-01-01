/*******************************************************************************
 *
 * \file
 * \brief Signed integer overflow checker implementation
 *
 * Author: Thomas Bailleux
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

#include <ikos/analyzer/checker/signed_int_overflow.hpp>

namespace ikos {
namespace analyzer {

SignedIntOverflowChecker::SignedIntOverflowChecker(Context& ctx)
    : IntOverflowCheckerBase(ctx) {}

CheckerName SignedIntOverflowChecker::name() const {
  return CheckerName::SignedIntOverflow;
}

const char* SignedIntOverflowChecker::description() const {
  return "Signed integer overflow checker";
}

void SignedIntOverflowChecker::check(ar::Statement* stmt,
                                     const value::AbstractDomain& inv,
                                     CallContext* call_context) {
  if (auto bin = dyn_cast< ar::BinaryOperation >(stmt)) {
    if (bin->op() == ar::BinaryOperation::SAdd ||
        bin->op() == ar::BinaryOperation::SSub ||
        bin->op() == ar::BinaryOperation::SMul ||
        bin->op() == ar::BinaryOperation::SDiv ||
        bin->op() == ar::BinaryOperation::SRem) {
      this->check_integer_overflow(bin, inv, call_context);
    }
  }
}

CheckKind SignedIntOverflowChecker::underflow_check_kind() const {
  return CheckKind::SignedIntUnderflow;
}

CheckKind SignedIntOverflowChecker::overflow_check_kind() const {
  return CheckKind::SignedIntOverflow;
}

} // end namespace analyzer
} // end namespace ikos
