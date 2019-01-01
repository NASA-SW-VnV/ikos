/*******************************************************************************
 *
 * \file
 * \brief StatementsTable implementation
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

#include <ikos/analyzer/database/table/statements.hpp>
#include <ikos/analyzer/util/source_location.hpp>

namespace ikos {
namespace analyzer {

StatementsTable::StatementsTable(sqlite::DbConnection& db,
                                 FilesTable& files,
                                 FunctionsTable& functions)
    : DatabaseTable(db,
                    "statements",
                    {{"id", sqlite::DbColumnType::Integer},
                     {"kind", sqlite::DbColumnType::Integer},
                     {"function_id", sqlite::DbColumnType::Integer},
                     {"file_id", sqlite::DbColumnType::Integer},
                     {"line", sqlite::DbColumnType::Integer},
                     {"column", sqlite::DbColumnType::Integer}},
                    {"function_id", "file_id"}),
      _files(files),
      _functions(functions),
      _row(db, "statements", 6) {}

sqlite::DbInt64 StatementsTable::insert(ar::Statement* stmt) {
  ikos_assert(stmt != nullptr);

  auto it = this->_map.find(stmt);
  if (it != this->_map.end()) {
    return it->second;
  }

  sqlite::DbInt64 id = this->_last_insert_id++;

  this->_row << id;
  this->_row << static_cast< sqlite::DbInt64 >(stmt->kind());

  ar::Code* code = stmt->parent()->code();
  ikos_assert(code->is_function_body());
  this->_row << this->_functions.insert(code->function());

  ikos_assert(stmt->has_frontend());
  SourceLocation loc = source_location(stmt);
  if (loc) {
    this->_row << this->_files.insert(loc.file());
    this->_row << static_cast< sqlite::DbInt64 >(loc.line());
    this->_row << static_cast< sqlite::DbInt64 >(loc.column());
  } else {
    this->_row << sqlite::null;
    this->_row << sqlite::null;
    this->_row << sqlite::null;
  }

  this->_row << sqlite::end_row;

  this->_map.try_emplace(stmt, id);
  return id;
}

} // end namespace analyzer
} // end namespace ikos
