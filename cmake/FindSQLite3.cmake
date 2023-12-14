###############################################################################
#
# Find SQLite 3 headers and libraries.
#
# Author: Maxime Arthaud
#
# Contact: ikos@lists.nasa.gov
#
# Notices:
#
# Copyright (c) 2018-2023 United States Government as represented by the
# Administrator of the National Aeronautics and Space Administration.
# All Rights Reserved.
#
# Disclaimers:
#
# No Warranty: THE SUBJECT SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY OF
# ANY KIND, EITHER EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING, BUT NOT LIMITED
# TO, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL CONFORM TO SPECIFICATIONS,
# ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
# OR FREEDOM FROM INFRINGEMENT, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL BE
# ERROR FREE, OR ANY WARRANTY THAT DOCUMENTATION, IF PROVIDED, WILL CONFORM TO
# THE SUBJECT SOFTWARE. THIS AGREEMENT DOES NOT, IN ANY MANNER, CONSTITUTE AN
# ENDORSEMENT BY GOVERNMENT AGENCY OR ANY PRIOR RECIPIENT OF ANY RESULTS,
# RESULTING DESIGNS, HARDWARE, SOFTWARE PRODUCTS OR ANY OTHER APPLICATIONS
# RESULTING FROM USE OF THE SUBJECT SOFTWARE.  FURTHER, GOVERNMENT AGENCY
# DISCLAIMS ALL WARRANTIES AND LIABILITIES REGARDING THIRD-PARTY SOFTWARE,
# IF PRESENT IN THE ORIGINAL SOFTWARE, AND DISTRIBUTES IT "AS IS."
#
# Waiver and Indemnity:  RECIPIENT AGREES TO WAIVE ANY AND ALL CLAIMS AGAINST
# THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS, AS WELL
# AS ANY PRIOR RECIPIENT.  IF RECIPIENT'S USE OF THE SUBJECT SOFTWARE RESULTS
# IN ANY LIABILITIES, DEMANDS, DAMAGES, EXPENSES OR LOSSES ARISING FROM SUCH
# USE, INCLUDING ANY DAMAGES FROM PRODUCTS BASED ON, OR RESULTING FROM,
# RECIPIENT'S USE OF THE SUBJECT SOFTWARE, RECIPIENT SHALL INDEMNIFY AND HOLD
# HARMLESS THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS,
# AS WELL AS ANY PRIOR RECIPIENT, TO THE EXTENT PERMITTED BY LAW.
# RECIPIENT'S SOLE REMEDY FOR ANY SUCH MATTER SHALL BE THE IMMEDIATE,
# UNILATERAL TERMINATION OF THIS AGREEMENT.
#
###############################################################################

if (NOT SQLITE3_FOUND)
  set(SQLITE3_ROOT "" CACHE PATH "Path to sqlite3 install directory")

  find_path(SQLITE3_INCLUDE_DIR
    NAMES sqlite3.h
    HINTS "${SQLITE3_ROOT}/include"
    DOC "Path to sqlite3 include directory"
  )

  find_library(SQLITE3_LIB
    NAMES sqlite3
    HINTS "${SQLITE3_ROOT}/lib"
    DOC "Path to sqlite3 library"
  )

  if (SQLITE3_INCLUDE_DIR AND SQLITE3_LIB)
    file(WRITE "${PROJECT_BINARY_DIR}/FindSQLite3Version.c" "
      #include <assert.h>
      #include <stdio.h>
      #include <string.h>
      #include <sqlite3.h>

      int main() {
      // The following assertion does not always hold on macs, due to a bug in
      // the sqlite3 setup shipped on Mac. So, we only check if the OS is not
      // Apple.
      #ifndef __APPLE__
        assert(strcmp(SQLITE_VERSION, sqlite3_libversion()) == 0);
      #endif
        printf(\"%s\", sqlite3_libversion());
        return 0;
      }
    ")

    try_run(
      RUN_RESULT
      COMPILE_RESULT
      "${PROJECT_BINARY_DIR}"
      "${PROJECT_BINARY_DIR}/FindSQLite3Version.c"
      CMAKE_FLAGS
        "-DINCLUDE_DIRECTORIES:STRING=${SQLITE3_INCLUDE_DIR}"
        "-DLINK_LIBRARIES:STRING=${SQLITE3_LIB}"
      COMPILE_OUTPUT_VARIABLE COMPILE_OUTPUT
      RUN_OUTPUT_VARIABLE SQLITE3_VERSION
    )

    if (NOT COMPILE_RESULT)
      message(FATAL_ERROR "error when trying to compile a program with SQLite3:\n${COMPILE_OUTPUT}")
    endif()
    if (RUN_RESULT)
      message(FATAL_ERROR "error when running a program linked with SQLite3:\n${SQLITE3_VERSION}")
    endif()
  endif()

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(SQLite3
    REQUIRED_VARS
      SQLITE3_INCLUDE_DIR
      SQLITE3_LIB
    VERSION_VAR
      SQLITE3_VERSION
    FAIL_MESSAGE
      "Could NOT find SQLite3. Please provide -DSQLITE3_ROOT=/path/to/sqlite3")
endif()
