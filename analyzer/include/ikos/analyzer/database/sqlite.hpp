/*******************************************************************************
 *
 * \file
 * \brief Simple wrapper classes for SQLite3 database
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

#pragma once

#include <memory>
#include <string>

#include <sqlite3.h>

#include <llvm/ADT/ArrayRef.h>

#include <ikos/analyzer/exception.hpp>
#include <ikos/analyzer/support/assert.hpp>
#include <ikos/analyzer/support/string_ref.hpp>

namespace ikos {
namespace analyzer {
namespace sqlite {

/// \brief Database error
class DbError : public analyzer::Exception {
private:
  /// \brief Error code
  int _code;

  /// \brief Context messagge
  ///
  /// See https://clang.llvm.org/extra/clang-tidy/checks/cert-err60-cpp.html
  std::shared_ptr< const std::string > _context;

  /// \brief Computed message for what() method
  ///
  /// See https://clang.llvm.org/extra/clang-tidy/checks/cert-err60-cpp.html
  std::shared_ptr< const std::string > _what;

public:
  /// \brief No default constructor
  DbError() = delete;

  /// \brief Constructor
  ///
  /// \param code SQLite error code
  explicit DbError(int code);

  /// \brief Constructor
  ///
  /// \param code SQLite error code
  /// \param context Context message
  DbError(int code, const std::string& context);

  /// \brief Copy constructor
  DbError(const DbError&) noexcept = default;

  /// \brief Move constructor
  DbError(DbError&&) noexcept = default;

  /// \brief Copy assignment operator
  DbError& operator=(const DbError&) noexcept = default;

  /// \brief Move assignment operator
  DbError& operator=(DbError&&) noexcept = default;

  /// \brief Return the error code
  int error_code() const noexcept { return this->_code; }

  /// \brief Return the context message
  const char* context() const noexcept { return this->_context->c_str(); }

  /// \brief SQLite error message
  const char* error_msg() const noexcept;

  /// \brief Get the explanatory string
  const char* what() const noexcept override;

  /// \brief Destructor
  ~DbError() override;

}; // end class DbError

/// \brief Integer type for SQLite
using DbInt64 = sqlite3_int64;

/// \brief Double type for SQLite
using DbDouble = double;

/// \brief Column type
enum class DbColumnType { Text, Integer, Real, Blob };

/// \brief Return the string representation of a column type
const char* column_type_str(DbColumnType t);

/// \brief Journal mode
enum class JournalMode { Delete, Truncate, Persist, Memory, WAL, Off };

/// \brief Synchronous mode
enum class SynchronousFlag { Off, Normal, Full, Extra };

/// \brief Commit policy for a DbConnection
enum class CommitPolicy {
  /// \brief Call begin_transaction() and commit_transaction() manually
  Manual = 0,

  /// \brief Automatically start new transactions every MaxRowsPerTransaction
  /// inserted rows
  Auto = 1,
};

/// \brief SQLite connection
class DbConnection {
public:
  /// \brief Maximum number of rows per transaction, in CommitPolicy::Auto
  static const int MaxRowsPerTransaction = 8192;

private:
  /// \brief Filename
  std::string _filename;

  /// \brief SQLite3 handle
  sqlite3* _handle = nullptr;

  /// \brief Commit policy
  CommitPolicy _commit_policy = CommitPolicy::Manual;

  /// \brief Number of inserted rows, in CommitPolicy::Auto
  std::size_t _inserted_rows = 0;

public:
  /// \brief No default constructor
  DbConnection() = delete;

  /// \brief Open a database connection
  explicit DbConnection(std::string filename);

  /// \brief No copy constructor
  DbConnection(const DbConnection&) = delete;

  /// \brief No move constructor
  DbConnection(DbConnection&&) = delete;

  /// \brief No copy assignment operator
  DbConnection& operator=(const DbConnection&) = delete;

  /// \brief No move assignment operator
  DbConnection& operator=(DbConnection&&) = delete;

  /// \brief Destructor
  ~DbConnection();

  /// \brief Return the database filename
  const std::string& filename() const { return this->_filename; }

  /// \brief Execute a SQL command
  void exec_command(const char* cmd);

  /// \brief Execute a SQL command
  void exec_command(const std::string& cmd);

  /// \brief Start a transaction
  void begin_transaction();

  /// \brief Commit a transaction
  void commit_transaction();

  /// \brief Rollback a transaction
  void rollback_transaction();

  /// \brief Change the commit policy
  void set_commit_policy(CommitPolicy policy);

  /// \brief Return the current commit policy
  CommitPolicy commit_policy() const { return this->_commit_policy; }

private:
  /// \brief Called upon a row insertion
  void row_inserted();

public:
  /// \brief Remove a table
  void drop_table(StringRef name);

  /// \brief Return the last inserted row ID
  DbInt64 last_insert_rowid() const;

  /// \brief Create a table with the given column names and types
  ///
  /// If a column is called "id", automatically mark it as a primary key.
  void create_table(
      StringRef name,
      llvm::ArrayRef< std::pair< StringRef, DbColumnType > > columns);

  /// \brief Create an index on the given column of the given table
  void create_index(StringRef index, StringRef table, StringRef column);

  /// \brief Set the journal mode
  void set_journal_mode(JournalMode mode);

  /// \brief Set the synchronous flag
  void set_synchronous_flag(SynchronousFlag flag);

  // friends
  friend class DbOstream;
  friend class DbIstream;

}; // end class DbConnection

/// \brief Stream-based interface for populating tables
class DbOstream {
private:
  /// \brief Database connection
  DbConnection& _db;

  /// \brief SQLite3 prepared statement
  sqlite3_stmt* _stmt = nullptr;

  /// \brief Number of columns
  int _columns;

  /// \brief Current number of column entered
  int _current_column = 1;

public:
  /// \brief No default constructor
  DbOstream() = delete;

  /// \brief Constructor
  ///
  /// \param db The database connection
  /// \param table_name The table name
  /// \param columns Number of columns
  DbOstream(DbConnection& db, StringRef table_name, int columns);

  /// \brief No copy constructor
  DbOstream(const DbOstream&) = delete;

  /// \brief No move constructor
  DbOstream(DbOstream&&) = delete;

  /// \brief No copy assignment operator
  DbOstream& operator=(const DbOstream&) = delete;

  /// \brief No move assignment operator
  DbOstream& operator=(DbOstream&&) = delete;

  /// \brief Destructor
  ~DbOstream();

public:
  /// \brief Insert a string
  void add(StringRef s);

  /// \brief Insert NULL
  void add_null();

  /// \brief Insert an integer
  void add(DbInt64 n);

  /// \brief Insert a double
  void add(DbDouble d);

  /// \brief Flush the row
  void flush();

  // friends
  friend DbOstream& end_row(DbOstream&);

}; // end class DbOstream

/// \brief Insert a string
inline DbOstream& operator<<(DbOstream& o, StringRef s) {
  o.add(s);
  return o;
}

/// \brief Mark the end of a row
inline DbOstream& end_row(DbOstream& o) {
  o.flush();
  return o;
}

/// \brief Insert NULL
inline DbOstream& null(DbOstream& o) {
  o.add_null();
  return o;
}

/// \brief Insert an integer
inline DbOstream& operator<<(DbOstream& o, DbInt64 n) {
  o.add(n);
  return o;
}

/// \brief Insert an double
inline DbOstream& operator<<(DbOstream& o, DbDouble d) {
  o.add(d);
  return o;
}

/// \brief Insert sqlite::end_row or sqlite::null
inline DbOstream& operator<<(DbOstream& o, DbOstream& (*m)(DbOstream&)) {
  if (m == &end_row) {
    o.flush();
  } else if (m == &null) {
    o.add_null();
  } else {
    ikos_unreachable("invalid function pointer argument");
  }
  return o;
}

/// \brief Stream-based interface for retrieving results of a SQL query
class DbIstream {
private:
  /// \brief Database connection
  DbConnection& _db;

  /// \brief SQLite3 prepared statement
  sqlite3_stmt* _stmt = nullptr;

  /// \brief SQL query
  std::string _query;

  /// \brief Are we done ready?
  bool _done = false;

  /// \brief Number of columns
  int _columns;

  /// \brief Current number of column retrieved
  int _current_column;

public:
  /// \brief No default constructor
  DbIstream() = delete;

  /// \brief Constructor
  ///
  /// \param db The database connection
  /// \param query SQL query
  DbIstream(DbConnection& db, std::string query);

  /// \brief No copy constructor
  DbIstream(const DbIstream&) = delete;

  /// \brief No copy constructor
  DbIstream(DbIstream&&) = delete;

  /// \brief No copy assignment operator
  DbIstream& operator=(const DbIstream&) = delete;

  /// \brief No move assignment operator
  DbIstream& operator=(DbIstream&&) = delete;

  /// \brief Destructor
  ~DbIstream();

  /// \brief Return the SQL query
  const std::string& query() const { return this->_query; }

  /// \brief Is the stream empty?
  bool empty() const { return _done; }

private:
  /// \brief Retrieve the next row
  void step();

  /// \brief Retrieve the next column
  void skip_column();

  /// \brief Read a string
  friend DbIstream& operator>>(DbIstream&, std::string&);

  /// \brief Read an integer
  friend DbIstream& operator>>(DbIstream&, DbInt64&);

  /// \brief Read a double
  friend DbIstream& operator>>(DbIstream&, DbDouble&);

}; // class DbIstream

} // end namespace sqlite
} // end namespace analyzer
} // end namespace ikos
