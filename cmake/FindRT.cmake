if (NOT RT_FOUND)
  find_library(RT_LIB
    NAMES rt
    DOC "Path to rt library"
  )

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(RT
    REQUIRED_VARS RT_LIB)
endif()
