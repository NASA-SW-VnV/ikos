/*******************************************************************************
 *
 * \file
 * \brief Implementation of the frontend checker
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

#include <ikos/ar/format/text.hpp>
#include <ikos/ar/semantic/statement.hpp>
#include <ikos/ar/verify/frontend.hpp>

namespace ikos {
namespace ar {

// Warning: it is very important to write:
// valid = f(err, ...) && valid;
// And not:
// valid = valid && f(err, ...);
//
// This is because f(err, ...) can have side effects (typically, writes to err),
// and with the second pattern, the compiler will remove the call to f() if
// valid is false because of short-circuiting.

bool FrontendVerifier::verify(Bundle* bundle, std::ostream& err) const {
  bool valid = true;
  for (auto it = bundle->global_begin(), et = bundle->global_end();
       it != et && (this->_all || valid);
       ++it) {
    valid = this->verify(*it, err) && valid;
  }
  for (auto it = bundle->function_begin(), et = bundle->function_end();
       it != et && (this->_all || valid);
       ++it) {
    valid = this->verify(*it, err) && valid;
  }
  return valid;
}

bool FrontendVerifier::verify(GlobalVariable* /*gv*/,
                              std::ostream& /*err*/) const {
  // Ignore statements in global variable initializer
  return true;
}

bool FrontendVerifier::verify(Function* f, std::ostream& err) const {
  return f->is_declaration() || this->verify(f->body(), err);
}

bool FrontendVerifier::verify(Code* code, std::ostream& err) const {
  bool valid = true;
  for (auto it = code->begin(), et = code->end();
       it != et && (this->_all || valid);
       ++it) {
    valid = this->verify(*it, err) && valid;
  }
  return valid;
}

bool FrontendVerifier::verify(BasicBlock* bb, std::ostream& err) const {
  bool valid = true;
  for (auto it = bb->begin(), et = bb->end(); it != et && (this->_all || valid);
       ++it) {
    Statement* stmt = *it;
    if (!stmt->has_frontend()) {
      err << "error: missing debug info for statement '";
      TextFormatter().format(err, stmt);
      err << "'\n";
      valid = false;
    }
  }
  return valid;
}

} // end namespace ar
} // end namespace ikos
