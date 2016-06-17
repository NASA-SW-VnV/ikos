/*******************************************************************************
 *
 * Wrapper to allow an analysis to write its results to an external
 * database
 *
 * Author: Jorge A. Navas
 *
 * Contributors: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2016 United States Government as represented by the
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

#include <iostream>
#include <memory>

#include <analyzer/utils/sqlite_api.hpp>

namespace analyzer {

// This class writes the results of the analysis into a database
class analysis_db {
public:
  typedef std::shared_ptr< analysis_db > db_ptr;

protected:
  std::string _analysis_name;
  sqlite::db_connection _db;

  analysis_db(const std::string& analysis_name, sqlite::db_connection db)
      : _analysis_name(analysis_name), _db(db) {}

public:
  virtual ~analysis_db() {}

  virtual void write(const std::string& kind,
                     const std::string& context,
                     const std::string& file,
                     sqlite::db_int line,
                     sqlite::db_int col,
                     sqlite::db_int stmt_uid,
                     const std::string& status) = 0;

}; // end analysis_db

class intra_db : public analysis_db {
private:
  std::string _results_table;

public:
  intra_db(const std::string& analysis_name, sqlite::db_connection db)
      : analysis_db(analysis_name, db),
        _results_table(analysis_name + "_results") {
    // Results table
    std::pair< std::string, sqlite::db_type > results_cols[6] =
        {{"safety_check", sqlite::db_type::Text},
         {"file", sqlite::db_type::Text},
         {"line", sqlite::db_type::Integer},
         {"column", sqlite::db_type::Integer},
         {"stmt_uid", sqlite::db_type::Integer},
         {"status", sqlite::db_type::Text}};
    _db.delete_table(_results_table);
    _db.create_table(_results_table, results_cols, 6);
    _db.create_index(analysis_name + "_results_index_1",
                     _results_table,
                     "safety_check");
    _db.create_index(analysis_name + "_results_index_2",
                     _results_table,
                     "file");
    _db.create_index(analysis_name + "_results_index_3",
                     _results_table,
                     "line");
    _db.create_index(analysis_name + "_results_index_4",
                     _results_table,
                     "column");
    _db.create_index(analysis_name + "_results_index_5",
                     _results_table,
                     "stmt_uid");
  }

  void write(const std::string& check,
             const std::string&,
             const std::string& file,
             sqlite::db_int line,
             sqlite::db_int col,
             sqlite::db_int stmt_uid,
             const std::string& status) {
    sqlite::db_ostream o(_db, _results_table, 6);
    o << check << file << line << col << stmt_uid << status << sqlite::end_row;
  }
}; // end class intra_db

class inter_db : public analysis_db {
private:
  std::string _results_table;

public:
  inter_db(const std::string& analysis_name, sqlite::db_connection db)
      : analysis_db(analysis_name, db),
        _results_table(analysis_name + "_results") {
    // Results table
    std::pair< std::string, sqlite::db_type > results_cols[7] =
        {{"safety_check", sqlite::db_type::Text},
         {"context", sqlite::db_type::Text},
         {"file", sqlite::db_type::Text},
         {"line", sqlite::db_type::Integer},
         {"column", sqlite::db_type::Integer},
         {"stmt_uid", sqlite::db_type::Integer},
         {"status", sqlite::db_type::Text}};
    _db.delete_table(_results_table);
    _db.create_table(_results_table, results_cols, 7);
    _db.create_index(analysis_name + "_results_index_1",
                     _results_table,
                     "safety_check");
    _db.create_index(analysis_name + "_results_index_2",
                     _results_table,
                     "file");
    _db.create_index(analysis_name + "_results_index_3",
                     _results_table,
                     "line");
    _db.create_index(analysis_name + "_results_index_4",
                     _results_table,
                     "column");
    _db.create_index(analysis_name + "_results_index_5",
                     _results_table,
                     "stmt_uid");
  }

  void write(const std::string& kind,
             const std::string& context,
             const std::string& file,
             sqlite::db_int line,
             sqlite::db_int col,
             sqlite::db_int stmt_uid,
             const std::string& status) {
    sqlite::db_ostream o(_db, _results_table, 7);
    o << kind << context << file << line << col << stmt_uid << status
      << sqlite::end_row;
  }
};
} // end namespace analyzer

#endif // ANALYZER_ANALYSIS_DB_HPP
