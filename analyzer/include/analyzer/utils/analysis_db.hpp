/*******************************************************************************
 *
 * Wrapper to allow an analysis to write its results to an external database
 *
 * Author: Maxime Arthaud
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

#ifndef ANALYZER_ANALYSIS_DB_HPP
#define ANALYZER_ANALYSIS_DB_HPP

#include <boost/noncopyable.hpp>

#include <analyzer/utils/json.hpp>
#include <analyzer/utils/json_helpers.hpp>
#include <analyzer/utils/sqlite_api.hpp>
#include <analyzer/analysis/common.hpp>
#include <analyzer/ar-wrapper/wrapper.hpp>

namespace analyzer {

class settings_table_t : public boost::noncopyable {
private:
  sqlite::db_connection _db;
  std::string _table_name;

public:
  settings_table_t(sqlite::db_connection db)
      : _db(db), _table_name("settings") {
    std::pair< std::string, sqlite::db_type > cols[2] =
        {{"name", sqlite::db_type::Text}, {"value", sqlite::db_type::Text}};
    _db.delete_table(_table_name);
    _db.create_table(_table_name, cols, 2);
    _db.create_index("_settings_index_1", _table_name, "name");
  }

  void write(const std::string& name, const char* value) {
    sqlite::db_ostream row(_db, _table_name, 2);
    row << name << value << sqlite::end_row;
  }

  void write(const std::string& name, const std::string& value) {
    sqlite::db_ostream row(_db, _table_name, 2);
    row << name << value << sqlite::end_row;
  }

  void write(const std::string& name, const json_node& value) {
    std::stringstream buf;
    buf << value;
    write(name, buf.str());
  }

  void write(const std::string& name, bool value) {
    write(name, value ? "true" : "false");
  }

}; // end class settings_table_t

class times_table_t : public boost::noncopyable {
private:
  sqlite::db_connection _db;
  std::string _table_name;

public:
  times_table_t(sqlite::db_connection db) : _db(db), _table_name("times") {
    std::pair< std::string, sqlite::db_type > cols[2] =
        {{"pass", sqlite::db_type::Text}, {"time", sqlite::db_type::Real}};
    _db.delete_table(_table_name);
    _db.create_table(_table_name, cols, 2);
    _db.create_index("_times_index_1", _table_name, "pass");
  }

  void write(const std::string& pass, sqlite::db_double time) {
    sqlite::db_ostream row(_db, _table_name, 2);
    row << pass << time << sqlite::end_row;
  }

}; // end class times_table_t

class results_table_t : public boost::noncopyable {
private:
  sqlite::db_connection _db;
  std::string _table_name;

public:
  results_table_t(sqlite::db_connection db) : _db(db), _table_name("results") {
    std::pair< std::string, sqlite::db_type > cols[9] =
        {{"safety_check", sqlite::db_type::Text},
         {"status", sqlite::db_type::Text},
         {"context", sqlite::db_type::Text},
         {"function", sqlite::db_type::Text},
         {"file", sqlite::db_type::Text},
         {"line", sqlite::db_type::Integer},
         {"column", sqlite::db_type::Integer},
         {"stmt_uid", sqlite::db_type::Integer},
         {"info", sqlite::db_type::Text}};
    _db.delete_table(_table_name);
    _db.create_table(_table_name, cols, 9);
    _db.create_index("_results_index_1", _table_name, "safety_check");
    _db.create_index("_results_index_2", _table_name, "status");
    _db.create_index("_results_index_3", _table_name, "context");
    _db.create_index("_results_index_4", _table_name, "function");
    _db.create_index("_results_index_5", _table_name, "file");
    _db.create_index("_results_index_6", _table_name, "line");
    _db.create_index("_results_index_7", _table_name, "column");
    _db.create_index("_results_index_8", _table_name, "stmt_uid");
  }

  void write(const std::string& kind,
             boost::optional< analysis_result > status,
             const std::string& call_context,
             const boost::optional< Function_ref >& function,
             const location& loc,
             sqlite::db_int stmt_uid,
             const json_dict& info = json_dict()) {
    sqlite::db_ostream row(_db, _table_name, 9);
    row << kind;
    if (status) {
      row << result_to_str(*status);
    } else {
      row << sqlite::null;
    }
    row << call_context;
    if (function) {
      row << ar::getName(*function);
    } else {
      row << sqlite::null;
    }
    row << loc.file;
    row << loc.line;
    row << loc.column;
    row << stmt_uid;
    if (!info.empty()) {
      std::stringstream buf;
      buf << info;
      row << buf.str();
    } else {
      row << sqlite::null;
    }
    row << sqlite::end_row;
  }

}; // end results_table_t

} // end namespace analyzer

#endif // ANALYZER_ANALYSIS_DB_HPP
