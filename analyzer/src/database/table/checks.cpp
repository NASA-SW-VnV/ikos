/*******************************************************************************
 *
 * \file
 * \brief ChecksTable implementation
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

#include <ikos/analyzer/database/table/checks.hpp>

namespace ikos {
namespace analyzer {

ChecksTable::ChecksTable(sqlite::DbConnection& db,
                         StatementsTable& statements,
                         OperandsTable& operands,
                         CallContextsTable& call_contexts)
    : DatabaseTable(db,
                    "checks",
                    {{"id", sqlite::DbColumnType::Integer},
                     {"kind", sqlite::DbColumnType::Integer},
                     {"checker", sqlite::DbColumnType::Integer},
                     {"status", sqlite::DbColumnType::Integer},
                     {"statement_id", sqlite::DbColumnType::Integer},
                     {"operands", sqlite::DbColumnType::Text},
                     {"call_context_id", sqlite::DbColumnType::Integer},
                     {"info", sqlite::DbColumnType::Text}},
                    {"statement_id", "call_context_id"}),
      _statements(statements),
      _operands(operands),
      _call_contexts(call_contexts),
      _row(db, "checks", 8) {}

void ChecksTable::insert(CheckKind kind,
                         CheckerName checker,
                         Result status,
                         ar::Statement* stmt,
                         CallContext* call_context,
                         llvm::ArrayRef< ar::Value* > operands,
                         const JsonDict& info) {
  sqlite::DbInt64 id = this->_last_insert_id++;

  this->_row << id;
  this->_row << static_cast< sqlite::DbInt64 >(kind);
  this->_row << static_cast< sqlite::DbInt64 >(checker);
  this->_row << static_cast< sqlite::DbInt64 >(status);
  this->_row << this->_statements.insert(stmt);
  if (!operands.empty() &&
      (status == Result::Warning || status == Result::Error)) {
    JsonList json_operands;
    for (auto operand : operands) {
      // Find operand number
      auto it = std::find(stmt->op_begin(), stmt->op_end(), operand);
      sqlite::DbInt64 operand_no = -1;
      if (it != stmt->op_end()) {
        operand_no = static_cast< sqlite::DbInt64 >(it - stmt->op_begin());
      }
      json_operands.add(JsonList{operand_no, this->_operands.insert(operand)});
    }
    this->_row << json_operands.str();
  } else {
    this->_row << sqlite::null;
  }
  this->_row << this->_call_contexts.insert(call_context);
  if (!info.empty()) {
    this->_row << info.str();
  } else {
    this->_row << sqlite::null;
  }
  this->_row << sqlite::end_row;
}

} // end namespace analyzer
} // end namespace ikos
