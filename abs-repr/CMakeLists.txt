#*******************************************************************************
#
# Author: Maxime Arthaud
#
# Contact: ikos@lists.nasa.gov
#
# Notices:
#
# Copyright (c) 2011-2017 United States Government as represented by the
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
#*****************************************************************************/

cmake_minimum_required(VERSION 2.8.12.2 FATAL_ERROR)

project(abstract-representation)

#
# Build settings
#

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message (FATAL_ERROR
    "In-source builds are not allowed. Please clean your source tree and try again.")
endif()

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE STRING "Install directory." FORCE)
endif()
message(STATUS "All libraries and binaries will be installed in ${CMAKE_INSTALL_PREFIX}")

#
# CMake policy
#

# Use @rpath on Mac OS X (requires MAC OS X >= 10.5)
if (POLICY CMP0042)
  cmake_policy(SET CMP0042 NEW)
else ()
  set(CMAKE_MACOSX_RPATH ON)
endif()

# For add_cxx_flag
if (POLICY CMP0054)
  cmake_policy(SET CMP0054 OLD)
endif()

# Do not build anything when running make install
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY TRUE)

#
# CMake utils
#

include(CheckCXXCompilerFlag)

macro(add_cxx_flag flag name)
  CHECK_CXX_COMPILER_FLAG(${flag} "CXX-SUPPORTS-${name}")
  if("CXX-SUPPORTS-${name}")
    add_definitions(${flag})
  else()
    message(FATAL_ERROR "${flag} flag is not supported by ${CMAKE_CXX_COMPILER}")
  endif()
endmacro()

#
# Dependency checks
#

# Add path for custom modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

set(CUSTOM_BOOST_ROOT "" CACHE PATH "Path to custom boost installation.")
if (CUSTOM_BOOST_ROOT)
  set(BOOST_ROOT "${CUSTOM_BOOST_ROOT}")
  set(Boost_NO_SYSTEM_PATHS "ON")
endif()

find_package(Boost 1.55.0 REQUIRED
             COMPONENTS program_options)
include_directories(${Boost_INCLUDE_DIRS})

find_package(GMP REQUIRED)
include_directories(${GMP_INCLUDE_DIR})
include_directories(${GMPXX_INCLUDE_DIR})

find_package(RT)
if (NOT RT_FOUND)
  set(RT_LIB "")
endif()

#
# Compiler flags
#

add_cxx_flag("-std=c++1y" "CXX14")
add_cxx_flag("-Wall" "WALL")
# warning flags from -Wextra
add_cxx_flag("-Wempty-body" "WEMPTY_BODY")
add_cxx_flag("-Wmissing-field-initializers" "WMISSING_FIELD_INITIALIZERS")
add_cxx_flag("-Wsign-compare" "WSIGN_COMPARE")
add_cxx_flag("-Wtype-limits" "WTYPE_LIMITS")
add_cxx_flag("-Wuninitialized" "WUNINITIALIZED")

#
# Targets
#

include_directories(include)

# arbos
add_library(arbos-api SHARED
  src/arbos/api/ar.cpp
  src/arbos/api/ar_factory.cpp
  src/arbos/api/ar_visitor.cpp
)
target_link_libraries(arbos-api ${Boost_LIBRARIES} ${GMP_LIB} ${GMPXX_LIB})
install(TARGETS arbos-api LIBRARY DESTINATION lib OPTIONAL)

add_executable(arbos src/arbos/arbos.cpp)
target_link_libraries(arbos arbos-api ${CMAKE_DL_LIBS} ${RT_LIB} pthread)
install(TARGETS arbos RUNTIME DESTINATION bin OPTIONAL)
set_target_properties(arbos PROPERTIES
                      INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# install headers
install(DIRECTORY include/
  DESTINATION include
  FILES_MATCHING
  PATTERN "*.hpp"
)

# passes
add_library(print SHARED src/passes/dumps/print.cpp)
target_link_libraries(print arbos-api)
install(TARGETS print DESTINATION lib OPTIONAL)

# example passes
add_subdirectory(src/passes/examples EXCLUDE_FROM_ALL)

# Hack so that "make install" also tries to install example passes
install(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/src/passes/examples/cmake_install.cmake")

#
# Regression tests
#

include(CTest)
enable_testing()

add_subdirectory(tests/regression/tests)
add_subdirectory(tests/regression/verifier-passes EXCLUDE_FROM_ALL)

# Hack so that "make install" also tries to install verifier passes
install(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/tests/regression/verifier-passes/cmake_install.cmake")

#
# Doxygen
#

find_package(Doxygen)
if (DOXYGEN_FOUND)
  configure_file(docs/doxygen/Doxyfile.in docs/Doxyfile @ONLY)
  configure_file(docs/doxygen/customdoxygen.css docs/customdoxygen.css @ONLY)
  configure_file(docs/doxygen/DoxygenLayout.xml docs/DoxygenLayout.xml @ONLY)
  add_custom_target(doxygen-arbos
    ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/docs/Doxyfile"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Generating ARBOS API documentation with Doxygen" VERBATIM
  )
  install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/docs/html" DESTINATION docs/arbos OPTIONAL)
endif()

#
# If it's the top level CMakeLists.txt, we add some aliases
#

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  add_custom_target(docs DEPENDS doxygen-arbos)
endif()
