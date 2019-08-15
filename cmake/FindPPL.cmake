###############################################################################
#
# Find PPL headers and libraries.
#
# Author: Maxime Arthaud
#
# Contact: ikos@lists.nasa.gov
#
# Notices:
#
# Copyright (c) 2018-2019 United States Government as represented by the
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
    function(run_ppl_config FLAG OUTPUT_VAR)
      execute_process(
        COMMAND "${PPL_CONFIG_EXECUTABLE}" "${FLAG}"
        RESULT_VARIABLE HAD_ERROR
        OUTPUT_VARIABLE ${OUTPUT_VAR}
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE
      )
      if (HAD_ERROR)
        message(FATAL_ERROR "ppl-config failed with status: ${HAD_ERROR}")
      endif()
      set(${OUTPUT_VAR} "${${OUTPUT_VAR}}" PARENT_SCOPE)
    endfunction()

    run_ppl_config("--includedir" PPL_CONFIG_INCLUDE_DIR)
    run_ppl_config("--libdir" PPL_CONFIG_LIB_DIR)

    list(APPEND PPL_INCLUDE_SEARCH_DIRS ${PPL_CONFIG_INCLUDE_DIR})
    list(APPEND PPL_LIB_SEARCH_DIRS ${PPL_CONFIG_LIB_DIR})
  endif()

  find_package(GMP)

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

  if (PPL_INCLUDE_DIR AND PPL_C_LIB AND GMP_FOUND)
    file(WRITE "${PROJECT_BINARY_DIR}/FindPPLVersion.c" "
      #include <stdio.h>
      #include <ppl_c.h>

      int main() {
        const char* version;
        ppl_initialize();
        ppl_version(&version);
        printf(\"%s\", version);
        return ppl_finalize();
      }
    ")

    try_run(
      RUN_RESULT
      COMPILE_RESULT
      "${PROJECT_BINARY_DIR}"
      "${PROJECT_BINARY_DIR}/FindPPLVersion.c"
      CMAKE_FLAGS
        "-DINCLUDE_DIRECTORIES:STRING=${PPL_INCLUDE_DIR};${GMP_INCLUDE_DIR}"
        "-DLINK_LIBRARIES:STRING=${PPL_C_LIB};${GMP_LIB}"
      COMPILE_OUTPUT_VARIABLE COMPILE_OUTPUT
      RUN_OUTPUT_VARIABLE PPL_VERSION
    )

    if (NOT COMPILE_RESULT)
      message(FATAL_ERROR "error when trying to compile a program with PPL:\n${COMPILE_OUTPUT}")
    endif()
    if (RUN_RESULT)
      message(FATAL_ERROR "error when running a program linked with PPL:\n${PPL_VERSION}")
    endif()
  endif()

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(PPL
    REQUIRED_VARS
      PPL_INCLUDE_DIR
      PPL_LIB
      PPL_C_LIB
      GMP_FOUND
    VERSION_VAR
      PPL_VERSION
    FAIL_MESSAGE
      "Could NOT find PPL. Please provide -DPPL_ROOT=/path/to/ppl")
endif()

set(PPL_LIBRARIES
  ${PPL_LIB}
  ${PPL_C_LIB}
)
