if (NOT RT_FOUND)
  find_library(RT_LIB
    NAMES rt
  )

  mark_as_advanced(RT_LIB)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(RT
    REQUIRED_VARS RT_LIB)
endif()
