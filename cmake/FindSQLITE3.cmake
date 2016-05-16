if (NOT SQLITE3_FOUND)
  set(SQLITE3_SEARCH_PATH "" CACHE PATH "Search path for sqlite3.")

  find_path(SQLITE3_INCLUDE_DIR
    NAMES sqlite3.h
    PATHS "${SQLITE3_SEARCH_PATH}/include"
  )

  find_library(SQLITE3_LIB
    NAMES sqlite3
    PATHS "${SQLITE3_SEARCH_PATH}/lib"
  )

  mark_as_advanced(SQLITE3_SEARCH_PATH SQLITE3_INCLUDE_DIR SQLITE3_LIB)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(SQLITE3
    REQUIRED_VARS SQLITE3_INCLUDE_DIR SQLITE3_LIB)
endif()
