/*******************************************************************************
 *
 * \file
 * \brief Implementation of SQLite3 wrapper
 *
 * Authors: Arnaud J. Venet
 *
 * Contributors: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2019 United States Government as represented by the
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

#include <sstream>

#include <ikos/core/support/compiler.hpp>

#include <ikos/analyzer/database/sqlite.hpp>

namespace ikos {
namespace analyzer {
namespace sqlite {

// DbError

/// \brief Return the error message for the given code
///
/// Taken from the SQLite online documentation.
static const char* error_code_message(int code) noexcept {
  switch (code) {
    case SQLITE_ERROR:
      return "SQL error or missing database";
    case SQLITE_INTERNAL:
      return "Internal logic error in SQLite";
    case SQLITE_PERM:
      return "Access permission denied";
    case SQLITE_ABORT:
      return "Callback routine requested an abort";
    case SQLITE_BUSY:
      return "The database file is locked";
    case SQLITE_LOCKED:
      return "A table in the database is locked";
    case SQLITE_NOMEM:
      return "A malloc() failed";
    case SQLITE_READONLY:
      return "Attempt to write a readonly database";
    case SQLITE_INTERRUPT:
      return "Operation terminated by sqlite3_interrupt()";
    case SQLITE_IOERR:
      return "Some kind of disk I/O error occurred";
    case SQLITE_CORRUPT:
      return "The database disk image is malformed";
    case SQLITE_NOTFOUND:
      return "Unknown opcode in sqlite3_file_control()";
    case SQLITE_FULL:
      return "Insertion failed because database is full";
    case SQLITE_CANTOPEN:
      return "Unable to open the database file";
    case SQLITE_PROTOCOL:
      return "Database lock protocol error";
    case SQLITE_EMPTY:
      return "Database is empty";
    case SQLITE_SCHEMA:
      return "The database schema changed";
    case SQLITE_TOOBIG:
      return "String or BLOB exceeds size limit";
    case SQLITE_CONSTRAINT:
      return "Abort due to constraint violation";
    case SQLITE_MISMATCH:
      return "Data type mismatch";
    case SQLITE_MISUSE:
      return "Library used incorrectly";
    case SQLITE_NOLFS:
      return "Uses OS features not supported on host";
    case SQLITE_AUTH:
      return "Authorization denied";
    case SQLITE_FORMAT:
      return "Auxiliary database format error";
    case SQLITE_RANGE:
      return "2nd parameter to sqlite3_bind out of range";
    case SQLITE_NOTADB:
      return "File opened that is not a database file";
    case SQLITE_ROW:
      return "sqlite3_step() has another row ready";
    case SQLITE_DONE:
      return "sqlite3_step() has finished executing";
    default:
      return "Unknown error code";
  }
}

DbError::DbError(int code)
    : _code(code),
      _context(std::make_shared< const std::string >()),
      _what(std::make_shared< const std::string >(
          "[" + std::to_string(code) + "] " + error_code_message(code))) {}

DbError::DbError(int code, const std::string& context)
    : _code(code),
      _context(std::make_shared< const std::string >(context)),
      _what(std::make_shared< const std::string >(context + "\n[" +
                                                  std::to_string(code) + "] " +
                                                  error_code_message(code))) {}

const char* DbError::error_msg() const noexcept {
  return error_code_message(this->_code);
}

const char* DbError::what() const noexcept {
  return this->_what->c_str();
}

DbError::~DbError() = default;

// DbColumnType

const char* column_type_str(DbColumnType t) {
  switch (t) {
    case DbColumnType::Text:
      return "TEXT";
    case DbColumnType::Integer:
      return "INTEGER";
    case DbColumnType::Real:
      return "REAL";
    case DbColumnType::Blob:
      return "BLOB";
    default:
      ikos_unreachable("unreachable");
  }
}

// DbConnection

DbConnection::DbConnection(std::string filename)
    : _filename(std::move(filename)) {
  int status = sqlite3_open_v2(this->_filename.c_str(),
                               &this->_handle,
                               SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                               nullptr);
  if (status != SQLITE_OK) {
    throw DbError(status,
                  "DbConnection: cannot open database: " + this->_filename);
  }
}

DbConnection::~DbConnection() {
  // The destructor shall not throw an exception. No error check.
  if (this->_commit_policy == CommitPolicy::Auto) {
    sqlite3_exec(this->_handle, "COMMIT", nullptr, nullptr, nullptr);
  }

  sqlite3_close(this->_handle);
}

void DbConnection::exec_command(const char* cmd) {
  ikos_assert_msg(cmd != nullptr, "cmd is null");

  int status = sqlite3_exec(this->_handle, cmd, nullptr, nullptr, nullptr);
  if (status != SQLITE_OK) {
    throw DbError(status, "DbConnection::exec_command(): " + std::string(cmd));
  }
}

void DbConnection::exec_command(const std::string& cmd) {
  this->exec_command(cmd.c_str());
}

void DbConnection::begin_transaction() {
  ikos_assert(this->_commit_policy == CommitPolicy::Manual);
  this->exec_command("BEGIN");
}

void DbConnection::commit_transaction() {
  ikos_assert(this->_commit_policy == CommitPolicy::Manual);
  this->exec_command("COMMIT");
}

void DbConnection::rollback_transaction() {
  ikos_assert(this->_commit_policy == CommitPolicy::Manual);
  this->exec_command("ROLLBACK");
}

void DbConnection::set_commit_policy(CommitPolicy policy) {
  if (this->_commit_policy == CommitPolicy::Auto) {
    this->exec_command("COMMIT");
    this->_inserted_rows = 0;
  }

  this->_commit_policy = policy;

  if (this->_commit_policy == CommitPolicy::Auto) {
    this->exec_command("BEGIN");
    this->_inserted_rows = 0;
  }
}

void DbConnection::row_inserted() {
  if (this->_commit_policy == CommitPolicy::Auto) {
    this->_inserted_rows++;

    if (this->_inserted_rows >= MaxRowsPerTransaction) {
      this->exec_command("COMMIT");
      this->_inserted_rows = 0;
      this->exec_command("BEGIN");
    }
  }
}

void DbConnection::drop_table(StringRef name) {
  std::string cmd("DROP TABLE IF EXISTS ");
  cmd += name;
  this->exec_command(cmd);
}

DbInt64 DbConnection::last_insert_rowid() const {
  return sqlite3_last_insert_rowid(this->_handle);
}

void DbConnection::create_table(
    StringRef name,
    llvm::ArrayRef< std::pair< StringRef, DbColumnType > > columns) {
  ikos_assert_msg(!columns.empty(), "columns is empty");

  std::string cmd("CREATE TABLE IF NOT EXISTS ");
  cmd += name;
  cmd += '(';
  for (auto it = columns.begin(), et = columns.end(); it != et;) {
    cmd += it->first;
    cmd += ' ';
    cmd += column_type_str(it->second);
    if (it->first == "id") {
      cmd += " PRIMARY KEY";
    }
    ++it;
    if (it != et) {
      cmd += ',';
    }
  }
  cmd += ')';
  this->exec_command(cmd.c_str());
}

void DbConnection::create_index(StringRef index,
                                StringRef table,
                                StringRef column) {
  std::string cmd("CREATE INDEX IF NOT EXISTS ");
  cmd += index;
  cmd += " ON ";
  cmd += table;
  cmd += '(';
  cmd += column;
  cmd += ')';
  this->exec_command(cmd.c_str());
}

void DbConnection::set_journal_mode(JournalMode mode) {
  switch (mode) {
    case JournalMode::Delete: {
      this->exec_command("PRAGMA journal_mode = DELETE");
    } break;
    case JournalMode::Truncate: {
      this->exec_command("PRAGMA journal_mode = TRUNCATE");
    } break;
    case JournalMode::Persist: {
      this->exec_command("PRAGMA journal_mode = PERSIST");
    } break;
    case JournalMode::Memory: {
      this->exec_command("PRAGMA journal_mode = MEMORY");
    } break;
    case JournalMode::WAL: {
      this->exec_command("PRAGMA journal_mode = WAL");
    } break;
    case JournalMode::Off: {
      this->exec_command("PRAGMA journal_mode = OFF");
    } break;
  }
}

void DbConnection::set_synchronous_flag(SynchronousFlag flag) {
  switch (flag) {
    case SynchronousFlag::Off: {
      this->exec_command("PRAGMA synchronous = OFF");
    } break;
    case SynchronousFlag::Normal: {
      this->exec_command("PRAGMA synchronous = NORMAL");
    } break;
    case SynchronousFlag::Full: {
      this->exec_command("PRAGMA synchronous = FULL");
    } break;
    case SynchronousFlag::Extra: {
      this->exec_command("PRAGMA synchronous = EXTRA");
    } break;
  }
}

// DbOstream

DbOstream::DbOstream(DbConnection& db, StringRef table_name, int columns)
    : _db(db), _columns(columns) {
  ikos_assert_msg(columns > 0, "invalid number of columns");

  // Create SQL command
  std::string insert("INSERT INTO ");
  insert += table_name;
  insert += " VALUES (";
  for (int i = 0; i < columns; i++) {
    insert += '?';
    insert += (i + 1 < columns) ? ',' : ')';
  }

  int status = sqlite3_prepare_v2(this->_db._handle,
                                  insert.c_str(),
                                  -1,
                                  &this->_stmt,
                                  nullptr);
  if (status != SQLITE_OK) {
    throw DbError(status,
                  "DbOstream: cannot populate " + table_name.to_string() +
                      " in database " + this->_db.filename());
  }
}

DbOstream::~DbOstream() {
  // The destructor shall not throw an exception. No error check is performed.
  sqlite3_finalize(_stmt);
}

void DbOstream::add(StringRef s) {
  ikos_assert(s.size() <=
              static_cast< std::size_t >(std::numeric_limits< int >::max()));

  int status = sqlite3_bind_text(this->_stmt,
                                 this->_current_column++,
                                 s.data(),
                                 static_cast< int >(s.size()),
                                 SQLITE_TRANSIENT);
  if (status != SQLITE_OK) {
    throw DbError(status, "DbOstream::add(StringRef)");
  }
}

void DbOstream::add_null() {
  int status = sqlite3_bind_null(this->_stmt, this->_current_column++);
  if (status != SQLITE_OK) {
    throw DbError(status, "DbOstream::add_null()");
  }
}

void DbOstream::add(DbInt64 n) {
  int status = sqlite3_bind_int64(this->_stmt, this->_current_column++, n);
  if (status != SQLITE_OK) {
    throw DbError(status, "DbOstream::add(DbInt64)");
  }
}

void DbOstream::add(DbDouble d) {
  int status = sqlite3_bind_double(this->_stmt, this->_current_column++, d);
  if (status != SQLITE_OK) {
    throw DbError(status, "DbOstream::add(DbDouble)");
  }
}

void DbOstream::flush() {
  ikos_assert_msg(this->_current_column == this->_columns + 1,
                  "incomplete row");
  ikos_ignore(this->_columns);

  int status = sqlite3_step(this->_stmt);
  if (status != SQLITE_DONE) {
    throw DbError(status, "DbOstream::flush(): step failed");
  }

  status = sqlite3_clear_bindings(this->_stmt);
  if (status != SQLITE_OK) {
    throw DbError(status, "DbOstream::flush(): clear bindings failed");
  }

  status = sqlite3_reset(this->_stmt);
  if (status != SQLITE_OK) {
    throw DbError(status, "DbOstream::flush(): reset failed");
  }

  this->_current_column = 1;
  this->_db.row_inserted();
}

// DbIstream

DbIstream::DbIstream(DbConnection& db, std::string query)
    : _db(db), _query(std::move(query)) {
  int status = sqlite3_prepare_v2(this->_db._handle,
                                  this->_query.c_str(),
                                  -1,
                                  &this->_stmt,
                                  nullptr);
  if (status != SQLITE_OK) {
    throw DbError(status,
                  "DbIstream: cannot prepare query '" + this->_query +
                      "' in database " + this->_db.filename());
  }

  this->step();

  this->_columns = sqlite3_column_count(_stmt);
  if (this->_columns == 0) {
    throw DbError(SQLITE_MISMATCH,
                  "DbIstream: malformed query '" + this->_query +
                      "': no column in result");
  }

  this->_current_column = 0;
}

DbIstream::~DbIstream() {
  // The destructor shall not throw an exception. No error check.
  sqlite3_finalize(this->_stmt);
}

void DbIstream::step() {
  int status = sqlite3_step(this->_stmt);
  if (status == SQLITE_DONE) {
    this->_done = true;
  } else if (status != SQLITE_ROW) {
    throw DbError(status, "DbIstream::step(): query " + this->_query);
  }
}

void DbIstream::skip_column() {
  this->_current_column++;
  if (!this->_done && (this->_current_column == this->_columns)) {
    this->step();
    this->_current_column = 0;
  }
}

DbIstream& operator>>(DbIstream& i, std::string& s) {
  if (i.empty()) {
    throw DbError(SQLITE_MISUSE,
                  "DbIstream::>>: no more data for query '" + i.query() + "'");
  } else {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    s.assign(reinterpret_cast< const char* >(
        sqlite3_column_text(i._stmt, i._current_column)));
    i.skip_column();
    return i;
  }
}

DbIstream& operator>>(DbIstream& i, DbInt64& n) {
  if (i.empty()) {
    throw DbError(SQLITE_MISUSE,
                  "DbIstream::>>: no more data for query '" + i.query() + "'");
  } else {
    n = sqlite3_column_int64(i._stmt, i._current_column);
    i.skip_column();
    return i;
  }
}

DbIstream& operator>>(DbIstream& i, DbDouble& d) {
  if (i.empty()) {
    throw DbError(SQLITE_MISUSE,
                  "DbIstream::>>: no more data for query '" + i.query() + "'");
  } else {
    d = sqlite3_column_double(i._stmt, i._current_column);
    i.skip_column();
    return i;
  }
}

} // end namespace sqlite
} // end namespace analyzer
} // end namespace ikos
