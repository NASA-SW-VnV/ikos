/*******************************************************************************
 *
 * \file
 * \brief CallContextsTable implementation
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

#include <ikos/analyzer/database/table/call_contexts.hpp>

namespace ikos {
namespace analyzer {

CallContextsTable::CallContextsTable(sqlite::DbConnection& db,
                                     FunctionsTable& functions,
                                     StatementsTable& statements)
    : DatabaseTable(db,
                    "call_contexts",
                    {{"id", sqlite::DbColumnType::Integer},
                     {"call_id", sqlite::DbColumnType::Integer},
                     {"function_id", sqlite::DbColumnType::Integer},
                     {"parent_id", sqlite::DbColumnType::Integer}},
                    {"call_id", "function_id", "parent_id"}),
      _functions(functions),
      _statements(statements),
      _row(db, "call_contexts", 4) {}

sqlite::DbInt64 CallContextsTable::insert(CallContext* call_context) {
  ikos_assert(call_context != nullptr);

  auto it = this->_map.find(call_context);
  if (it != this->_map.end()) {
    return it->second;
  }

  // Insert the parent first
  sqlite::DbInt64 parent_id = 0;
  if (call_context->has_parent()) {
    parent_id = this->insert(call_context->parent());
  }

  // Insert row
  sqlite::DbInt64 id = this->_last_insert_id++;

  this->_row << id;
  if (call_context->empty()) {
    this->_row << sqlite::null;
    this->_row << sqlite::null;
    this->_row << sqlite::null;
  } else {
    // call_id
    ar::CallBase* call = call_context->call();
    this->_row << this->_statements.insert(call);

    // function_id
    ar::Code* code = call->code();
    ikos_assert(code->is_function_body());
    this->_row << this->_functions.insert(code->function());

    // parent_id
    this->_row << parent_id;
  }
  this->_row << sqlite::end_row;

  this->_map.try_emplace(call_context, id);
  return id;
}

} // end namespace analyzer
} // end namespace ikos
