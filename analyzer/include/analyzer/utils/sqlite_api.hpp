/*******************************************************************************
 *
 * Simple wrapper classes for SQLite3 database
 *
 * Authors: Arnaud J. Venet
 *
 * Contributors: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2017 United States Government as represented by the
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

#ifndef ANALYZER_SQLITE_API_HPP
#define ANALYZER_SQLITE_API_HPP

#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include <sqlite3.h>

namespace sqlite {

class db_error {
private:
  int _code;
  std::string _context;

private:
  db_error();

public:
  db_error(int code) : _code(code), _context("???") {}

  db_error(int code, const std::string& context)
      : _code(code), _context(context) {}

  int error_code() const { return _code; }

  const std::string& context() const { return _context; }

  // Eror messages taken from the SQLite online documentation (www.sqlite.org)
  std::string error_msg() const {
    switch (_code) {
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

  friend std::ostream& operator<<(std::ostream& o, const db_error& e) {
    o << e.context() << std::endl
      << e.error_msg() << " [" << e.error_code() << "]";
    return o;
  }
}; // class db_error

typedef sqlite3_int64 db_int;
typedef double db_double;

enum class db_type { Text, Integer, Real, Blob };

std::ostream& operator<<(std::ostream& o, db_type t) {
  switch (t) {
    case db_type::Text:
      o << "TEXT";
      break;
    case db_type::Integer:
      o << "INTEGER";
      break;
    case db_type::Real:
      o << "REAL";
      break;
    case db_type::Blob:
      o << "BLOB";
      break;
  }
  return o;
}

/*
 * SQLite connection
 */

class DBConnection {
  friend class db_ostream;
  friend class db_istream;

private:
  std::string _name;
  sqlite3* _handle;

private:
  DBConnection();
  DBConnection(const DBConnection&);
  DBConnection& operator=(DBConnection&);

public:
  DBConnection(const std::string& name) : _name(name) {
    int status = sqlite3_open_v2(_name.c_str(),
                                 &_handle,
                                 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                                 NULL);
    if (status != SQLITE_OK) {
      throw db_error(status, "DBConnection: cannot open database: " + name);
    }
  }

  ~DBConnection() {
    // The destructor shall not throw an exception. No error check.
    sqlite3_close(_handle);
  }

  void execCommand(const char* cmd) {
    if (!cmd) {
      throw db_error(SQLITE_MISUSE,
                     "DBConnection::execCommand: NULL pointer argument");
    }
    int status = sqlite3_exec(_handle, cmd, NULL, NULL, NULL);
    if (status != SQLITE_OK) {
      throw db_error(status, "DBConnection::execCommand: " + std::string(cmd));
    }
  }

  void execCommand(const std::string& cmd) { execCommand(cmd.c_str()); }

  void beginTransaction() { execCommand("BEGIN TRANSACTION"); }

  void endTransaction() { execCommand("COMMIT"); }

  void deleteTable(const std::string& name) {
    std::string cmd("DROP TABLE IF EXISTS ");
    cmd += name;
    execCommand(cmd);
  }

  db_int lastInsertRowid() const { return sqlite3_last_insert_rowid(_handle); }

  void createTable(const std::string& name,
                   const std::string* columns,
                   int ncols) {
    if (!columns || ncols <= 0) {
      throw db_error(SQLITE_MISUSE,
                     "DBConnection::createTable: incorrect column "
                     "specification");
    }
    std::ostringstream cmd;
    cmd << "CREATE TABLE IF NOT EXISTS " << name << "(";
    for (int i = 0; i < ncols; i++) {
      cmd << columns[i];
      if (ncols - i > 1) {
        cmd << ", ";
      }
    }
    cmd << ")";
    execCommand(cmd.str());
  }

  void createTable(const std::string& name,
                   const std::pair< std::string, db_type >* columns,
                   int ncols) {
    if (!columns || ncols <= 0) {
      throw db_error(SQLITE_MISUSE,
                     "DBConnection::createTable: incorrect column "
                     "specification");
    }
    std::ostringstream cmd;
    cmd << "CREATE TABLE IF NOT EXISTS " << name << "(";
    for (int i = 0; i < ncols; i++) {
      cmd << columns[i].first << " " << columns[i].second;
      if (ncols - i > 1) {
        cmd << ", ";
      }
    }
    cmd << ")";
    execCommand(cmd.str());
  }

  void createIndex(const std::string& index,
                   const std::string& table,
                   const std::string& column) {
    std::ostringstream cmd;
    cmd << "CREATE INDEX IF NOT EXISTS " << index << " ON " << table << "("
        << column << ")";
    execCommand(cmd.str());
  }

}; // class DBConnection

class db_connection {
public:
  typedef std::shared_ptr< DBConnection > db_connection_ptr;

public:
  db_connection_ptr db;

public:
  db_connection(const std::string& name)
      : db(std::make_shared< DBConnection >(name)) {}

  void exec_command(const char* cmd) { this->db->execCommand(cmd); }

  void exec_command(const std::string& cmd) { this->db->execCommand(cmd); }

  void begin_transaction() { this->db->beginTransaction(); }

  void end_transaction() { this->db->endTransaction(); }

  void delete_table(const std::string& name) { this->db->deleteTable(name); }

  db_int last_insert_rowid() const { return this->db->lastInsertRowid(); }

  void create_table(const std::string& name,
                    const std::string* columns,
                    int ncols) {
    this->db->createTable(name, columns, ncols);
  }

  void create_table(const std::string& name,
                    const std::pair< std::string, db_type >* columns,
                    int ncols) {
    this->db->createTable(name, columns, ncols);
  }

  void create_index(const std::string& index,
                    const std::string& table,
                    const std::string& column) {
    this->db->createIndex(index, table, column);
  }

}; // class db_connection

/*
 * Simple stream-based interface for populating tables.
 */

class db_ostream {
  friend db_ostream& end_row(db_ostream&);

public:
  static const int MAX_ROWS_PER_TRANSACTION = 1024;

private:
  db_connection _db;
  std::string _name;
  int _columns;
  int _stored_rows;
  int _current_column;
  bool _in_transaction;
  std::string _cmd;
  sqlite3_stmt* _stmt;

private:
  db_ostream();
  db_ostream(const db_ostream&);
  db_ostream& operator=(const db_ostream&);

  void reset() {
    _db.end_transaction();
    _stored_rows = 0;
    _db.begin_transaction();
  }

public:
  db_ostream(db_connection db,
             const std::string& name,
             int columns,
             bool in_transaction = true)
      : _db(db),
        _name(name),
        _columns(columns),
        _stored_rows(0),
        _current_column(1),
        _in_transaction(in_transaction) {
    if (!_db.db) {
      throw db_error(SQLITE_MISUSE,
                     "db_ostream: uninitialized database connection");
    }
    if (_columns <= 0) {
      throw db_error(SQLITE_MISUSE, "db_ostream: incorrect number of columns");
    }
    if (_in_transaction) {
      _db.begin_transaction();
    }
    std::ostringstream insert;
    insert << "INSERT INTO " << name << " VALUES (";
    for (int i = 0; i < columns; i++) {
      insert << "?" << ((columns - i > 1) ? ", " : ")");
    }
    _cmd = insert.str();
    int status =
        sqlite3_prepare_v2(_db.db->_handle, _cmd.c_str(), -1, &_stmt, NULL);
    if (status != SQLITE_OK) {
      throw db_error(status,
                     "db_ostream: cannot populate " + name + " in database " +
                         _db.db->_name);
    }
  }

  ~db_ostream() {
    // The destructor shall not throw an exception. No error check is performed.
    sqlite3_finalize(_stmt);
    if (_in_transaction) {
      sqlite3_exec(_db.db->_handle, "COMMIT", NULL, NULL, NULL);
    }
  }

  void add(const char* s) {
    if (!s) {
      throw db_error(SQLITE_MISUSE, "db_ostream::add: NULL pointer argument");
    }
    int status =
        sqlite3_bind_text(_stmt, _current_column++, s, -1, SQLITE_TRANSIENT);
    if (status != SQLITE_OK) {
      throw db_error(status, "db_ostream::add const char*");
    }
  }

  void add(const std::string& s) { add(s.c_str()); }

  void add_null() {
    int status = sqlite3_bind_null(_stmt, _current_column++);
    if (status != SQLITE_OK) {
      throw db_error(status, "db_ostream::add null");
    }
  }

  void add(db_int n) {
    int status = sqlite3_bind_int64(_stmt, _current_column++, n);
    if (status != SQLITE_OK) {
      throw db_error(status, "db_ostream::add int");
    }
  }

  void add(db_double d) {
    int status = sqlite3_bind_double(_stmt, _current_column++, d);
    if (status != SQLITE_OK) {
      throw db_error(status, "db_ostream::add double");
    }
  }

  void flush() {
    if (_current_column < _columns) {
      throw db_error(SQLITE_MISMATCH, "db_ostream: incomplete row");
    } else {
      int status = sqlite3_step(_stmt);
      if (status != SQLITE_DONE) {
        throw db_error(status, "db_ostream::flush: step");
      }
      status = sqlite3_clear_bindings(_stmt);
      if (status != SQLITE_OK) {
        throw db_error(status, "db_ostream::flush: clear bindings");
      }
      status = sqlite3_reset(_stmt);
      if (status != SQLITE_OK) {
        throw db_error(status, "db_ostream::flush: reset");
      }
      _current_column = 1;
      _stored_rows++;
      if (_in_transaction && _stored_rows >= MAX_ROWS_PER_TRANSACTION) {
        reset();
      }
    }
  }

}; // class db_ostream

inline db_ostream& operator<<(db_ostream& o, const char* s) {
  o.add(s);
  return o;
}

inline db_ostream& operator<<(db_ostream& o, const std::string& s) {
  o.add(s);
  return o;
}

inline db_ostream& end_row(db_ostream& o) {
  o.flush();
  return o;
}

inline db_ostream& null(db_ostream& o) {
  o.add_null();
  return o;
}

inline db_ostream& operator<<(db_ostream& o, db_int n) {
  o.add(n);
  return o;
}

inline db_ostream& operator<<(db_ostream& o, db_double d) {
  o.add(d);
  return o;
}

inline db_ostream& operator<<(db_ostream& o, db_ostream& (*m)(db_ostream&)) {
  if (m == &end_row) {
    o.flush();
  } else if (m == &null) {
    o.add_null();
  } else {
    throw db_error(SQLITE_MISUSE,
                   "db_ostream(operator <<): incorrect function pointer "
                   "argument");
  }
  return o;
}

/*
 * Simple stream-based interface for retrieving the results of a query.
 */

class db_istream {
  friend db_istream& operator>>(db_istream&, std::string&);
  friend db_istream& operator>>(db_istream&, db_int&);
  friend db_istream& operator>>(db_istream&, db_double&);

public:
  db_connection _db;
  std::string _query;
  bool _done;
  int _columns;
  int _current_column;
  sqlite3_stmt* _stmt;

private:
  db_istream();
  db_istream(const db_istream&);
  db_istream& operator=(const db_istream&);

  void step() {
    int status = sqlite3_step(_stmt);
    if (status == SQLITE_DONE) {
      _done = true;
    } else if (status != SQLITE_ROW) {
      throw db_error(status, "TableInputStream::step: query '" + _query + "'");
    }
  }

  void skipColumn() {
    _current_column++;
    if (!_done && (_current_column == _columns)) {
      step();
      _current_column = 0;
    }
  }

public:
  db_istream(db_connection db, const std::string& query)
      : _db(db), _query(query), _done(false) {
    if (!_db.db) {
      throw db_error(SQLITE_MISUSE,
                     "db_istream::constructor: uninitialized database "
                     "connection");
    }
    int status =
        sqlite3_prepare_v2(_db.db->_handle, _query.c_str(), -1, &_stmt, NULL);
    if (status != SQLITE_OK) {
      throw db_error(status,
                     "TableInputStream: cannot prepare query '" + query +
                         "' in database " + _db.db->_name);
    }
    step();
    _columns = sqlite3_column_count(_stmt);
    if (_columns == 0) {
      throw db_error(SQLITE_MISMATCH,
                     "db_istream: malformed query '" + query +
                         "': no column in result");
    }
    _current_column = 0;
  }

  ~db_istream() {
    // The destructor shall not throw an exception. No error check.
    sqlite3_finalize(_stmt);
  }

  bool empty() { return _done; }

}; // class db_istream

inline db_istream& operator>>(db_istream& i, std::string& s) {
  if (i.empty()) {
    throw db_error(SQLITE_MISUSE,
                   "db_istream::>>: no more data for query '" + i._query + "'");
  } else {
    s.assign(reinterpret_cast< const char* >(
        sqlite3_column_text(i._stmt, i._current_column)));
    i.skipColumn();
    return i;
  }
}

inline db_istream& operator>>(db_istream& i, db_int& n) {
  if (i.empty()) {
    throw db_error(SQLITE_MISUSE,
                   "db_istream::>>: no more data for query '" + i._query + "'");
  } else {
    n = sqlite3_column_int64(i._stmt, i._current_column);
    i.skipColumn();
    return i;
  }
}

inline db_istream& operator>>(db_istream& i, db_double& d) {
  if (i.empty()) {
    throw db_error(SQLITE_MISUSE,
                   "db_istream::>>: no more data for query '" + i._query + "'");
  } else {
    d = sqlite3_column_double(i._stmt, i._current_column);
    i.skipColumn();
    return i;
  }
}

} // end namespace

#endif // ANALYZER_SQLITE_API_HPP
