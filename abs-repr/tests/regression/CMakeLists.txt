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

project(abstract-representation-regression-tests)

#
# Build settings
#

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message (FATAL_ERROR
    "In-source builds are not allowed. Please clean your source tree and try again.")
endif()

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
endif()

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE STRING "Install directory." FORCE)
endif()
message(STATUS "All libraries and binaries will be installed in ${CMAKE_INSTALL_PREFIX}")

#
# CMake policy
#

if (POLICY CMP0054)
  cmake_policy(SET CMP0054 OLD) # For add_cxx_flag
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
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake")

set(CUSTOM_BOOST_ROOT "" CACHE PATH "Path to custom boost installation.")
if (CUSTOM_BOOST_ROOT)
  set(BOOST_ROOT "${CUSTOM_BOOST_ROOT}")
  set(Boost_NO_SYSTEM_PATHS "ON")
endif()

find_package(Boost 1.55.0 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(GMP REQUIRED)
include_directories(${GMP_INCLUDE_DIR})
include_directories(${GMPXX_INCLUDE_DIR})

find_package(ARBOS REQUIRED)
get_filename_component(ARBOS_LIB_DIR ${ARBOS_LIB} DIRECTORY)
include_directories(${ARBOS_INCLUDE_DIR})
link_directories(${ARBOS_LIB_DIR})

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

include(CTest)
enable_testing()

add_subdirectory(verifier-passes)
add_subdirectory(tests)
