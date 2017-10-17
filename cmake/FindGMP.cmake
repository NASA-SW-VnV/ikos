if (NOT GMP_FOUND)
  set(GMP_ROOT "" CACHE PATH "Path to gmp install directory.")

  find_path(GMP_INCLUDE_DIR
    NAMES gmp.h
    HINTS "${GMP_ROOT}/include"
  )

  find_library(GMP_LIB
    NAMES gmp
    HINTS "${GMP_ROOT}/lib"
  )

  find_path(GMPXX_INCLUDE_DIR
    NAMES gmpxx.h
    HINTS "${GMP_ROOT}/include"
  )

  find_library(GMPXX_LIB
    NAMES gmpxx
    HINTS "${GMP_ROOT}/lib"
  )

  mark_as_advanced(GMP_ROOT GMP_INCLUDE_DIR GMP_LIB GMPXX_INCLUDE_DIR GMPXX_LIB)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(GMP
    REQUIRED_VARS GMP_INCLUDE_DIR GMP_LIB GMPXX_INCLUDE_DIR GMPXX_LIB
    FAIL_MESSAGE "Could NOT find GMP. Please provide -DGMP_ROOT=<gmp-directory>")
endif()
