if (NOT MPFR_FOUND)
  set(MPFR_ROOT "" CACHE PATH "Path to mpfr install directory.")

  find_path(MPFR_INCLUDE_DIR
    NAMES mpfr.h
    HINTS "${MPFR_ROOT}/include"
  )

  find_library(MPFR_LIB
    NAMES mpfr
    HINTS "${MPFR_ROOT}/lib"
  )

  # Set MPFR_FIND_VERSION to 1.0.0 if no minimum version is specified
  if (NOT MPFR_FIND_VERSION)
    if (NOT MPFR_FIND_VERSION_MAJOR)
      set(MPFR_FIND_VERSION_MAJOR 1)
    endif()
    if (NOT MPFR_FIND_VERSION_MINOR)
      set(MPFR_FIND_VERSION_MINOR 0)
    endif()
    if (NOT MPFR_FIND_VERSION_PATCH)
      set(MPFR_FIND_VERSION_PATCH 0)
    endif()

    set(MPFR_FIND_VERSION "${MPFR_FIND_VERSION_MAJOR}.${MPFR_FIND_VERSION_MINOR}.${MPFR_FIND_VERSION_PATCH}")
  endif()

  if (MPFR_INCLUDE_DIR)
    # Set MPFR_VERSION
    file(READ "${MPFR_INCLUDE_DIR}/mpfr.h" _mpfr_version_header)

    string(REGEX MATCH "define[ \t]+MPFR_VERSION_MAJOR[ \t]+([0-9]+)" _mpfr_major_version_match "${_mpfr_version_header}")
    set(MPFR_MAJOR_VERSION "${CMAKE_MATCH_1}")
    string(REGEX MATCH "define[ \t]+MPFR_VERSION_MINOR[ \t]+([0-9]+)" _mpfr_minor_version_match "${_mpfr_version_header}")
    set(MPFR_MINOR_VERSION "${CMAKE_MATCH_1}")
    string(REGEX MATCH "define[ \t]+MPFR_VERSION_PATCHLEVEL[ \t]+([0-9]+)" _mpfr_patchlevel_version_match "${_mpfr_version_header}")
    set(MPFR_PATCHLEVEL_VERSION "${CMAKE_MATCH_1}")

    set(MPFR_VERSION "${MPFR_MAJOR_VERSION}.${MPFR_MINOR_VERSION}.${MPFR_PATCHLEVEL_VERSION}")

    # Check whether found version exceeds minimum version
    if (${MPFR_VERSION} VERSION_LESS ${MPFR_FIND_VERSION})
      set(MPFR_VERSION_OK FALSE)
      message(STATUS "MPFR version ${MPFR_VERSION} found in ${MPFR_INCLUDE_DIR}, "
                     "but at least version ${MPFR_FIND_VERSION} is required")
    else()
      set(MPFR_VERSION_OK TRUE)
    endif()
  endif()

  mark_as_advanced(MPFR_ROOT MPFR_INCLUDE_DIR MPFR_LIB)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(MPFR
    REQUIRED_VARS MPFR_INCLUDE_DIR MPFR_LIB MPFR_VERSION_OK
    FAIL_MESSAGE "Could NOT find MPFR. Please provide -DMPFR_ROOT=<mpfr-directory>")
endif()
