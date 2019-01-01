/*******************************************************************************
 *
 * \file
 * \brief Memory locations database table
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

#pragma once

#include <llvm/ADT/DenseMap.h>

#include <ikos/analyzer/analysis/memory_location.hpp>
#include <ikos/analyzer/database/table.hpp>
#include <ikos/analyzer/database/table/call_contexts.hpp>
#include <ikos/analyzer/database/table/functions.hpp>
#include <ikos/analyzer/database/table/statements.hpp>
#include <ikos/analyzer/json/json.hpp>

namespace ikos {
namespace analyzer {

/// \brief Memory locations table
class MemoryLocationsTable : public DatabaseTable {
private:
  /// \brief Functions table
  FunctionsTable& _functions;

  /// \brief Statements table
  StatementsTable& _statements;

  /// \brief Call contexts table
  CallContextsTable& _call_contexts;

  /// \brief Database output stream
  sqlite::DbOstream _row;

  /// \brief Map from MemoryLocation* to id
  llvm::DenseMap< MemoryLocation*, sqlite::DbInt64 > _map;

  /// \brief Last inserted id
  sqlite::DbInt64 _last_insert_id = 0;

public:
  /// \brief Constructor
  explicit MemoryLocationsTable(sqlite::DbConnection& db,
                                FunctionsTable& functions,
                                StatementsTable& statements,
                                CallContextsTable& call_contexts);

  /// \brief Insert the given memory location in the database and return the id
  sqlite::DbInt64 insert(MemoryLocation* mem_loc);

  /// \brief Return the json info for the given memory location
  JsonDict info(MemoryLocation* mem_loc);

}; // end class MemoryLocationsTable

} // end namespace analyzer
} // end namespace ikos
