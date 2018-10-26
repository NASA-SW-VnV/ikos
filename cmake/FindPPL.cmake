if (NOT PPL_FOUND)
  set(PPL_ROOT "" CACHE PATH "Path to ppl install directory")

  set(PPL_INCLUDE_SEARCH_DIRS "")
  set(PPL_LIB_SEARCH_DIRS "")

  if (PPL_ROOT)
    list(APPEND PPL_INCLUDE_SEARCH_DIRS "${PPL_ROOT}/include")
    list(APPEND PPL_LIB_SEARCH_DIRS "${PPL_ROOT}/lib")
  endif()

  # Try to use ppl-config if available
  find_program(PPL_CONFIG_EXECUTABLE CACHE NAMES ppl-config DOC "Path to ppl-config binary")

  if (PPL_CONFIG_EXECUTABLE)
    execute_process(
      COMMAND ${PPL_CONFIG_EXECUTABLE} --includedir
      RESULT_VARIABLE HAD_ERROR
      OUTPUT_VARIABLE PPL_CONFIG_INCLUDE_DIR
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (HAD_ERROR)
      message(FATAL_ERROR "ppl-config failed with status: ${HAD_ERROR}")
    endif()

    execute_process(
      COMMAND ${PPL_CONFIG_EXECUTABLE} --libdir
      RESULT_VARIABLE HAD_ERROR
      OUTPUT_VARIABLE PPL_CONFIG_LIB_DIR
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (HAD_ERROR)
      message(FATAL_ERROR "ppl-config failed with status: ${HAD_ERROR}")
    endif()

    list(APPEND PPL_INCLUDE_SEARCH_DIRS ${PPL_CONFIG_INCLUDE_DIR})
    list(APPEND PPL_LIB_SEARCH_DIRS ${PPL_CONFIG_LIB_DIR})
  endif()

  find_path(PPL_INCLUDE_DIR
    NAMES ppl_c.h
    HINTS ${PPL_INCLUDE_SEARCH_DIRS}
    DOC "Path to ppl include directory"
  )

  find_library(PPL_LIB
    NAMES ppl
    HINTS ${PPL_LIB_SEARCH_DIRS}
    DOC "Path to ppl library"
  )

  find_library(PPL_C_LIB
    NAMES ppl_c
    HINTS ${PPL_LIB_SEARCH_DIRS}
    DOC "Path to ppl_c library"
  )

  set(PPL_LIBRARIES ${PPL_LIB} ${PPL_C_LIB})
  mark_as_advanced(PPL_LIBRARIES)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(PPL
    REQUIRED_VARS
      PPL_INCLUDE_DIR
      PPL_LIB
      PPL_C_LIB
    FAIL_MESSAGE
      "Could NOT find PPL. Please provide -DPPL_ROOT=/path/to/ppl")
endif()
