if (NOT SQLITE3_FOUND)
  set(SQLITE3_ROOT "" CACHE PATH "Path to sqlite3 install directory.")

  find_path(SQLITE3_INCLUDE_DIR
    NAMES sqlite3.h
    HINTS "${SQLITE3_ROOT}/include"
  )

  find_library(SQLITE3_LIB
    NAMES sqlite3
    HINTS "${SQLITE3_ROOT}/lib"
  )

  mark_as_advanced(SQLITE3_ROOT SQLITE3_INCLUDE_DIR SQLITE3_LIB)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(SQLITE3
    REQUIRED_VARS SQLITE3_INCLUDE_DIR SQLITE3_LIB
    FAIL_MESSAGE "Could NOT find SQLITE3. Please provide -DSQLITE3_ROOT=<sqlite3-directory>")
endif()
