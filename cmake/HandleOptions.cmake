# Check the build type
set(CMAKE_BUILD_TYPES "Debug;Release;RelWithDebInfo;MinSizeRel")
if (NOT (CMAKE_BUILD_TYPE IN_LIST CMAKE_BUILD_TYPES))
  message(FATAL_ERROR "Unsupported build type: ${CMAKE_BUILD_TYPE}")
endif()

# Option to enable assertions
if (NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
  option(ENABLE_ASSERTIONS "Enable assertions" OFF)
else()
  option(ENABLE_ASSERTIONS "Enable assertions" ON)
endif()

if (ENABLE_ASSERTIONS)
  # On non-Debug builds, cmake automatically defines NDEBUG, so we undefine it.
  if (NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    add_definitions("-UNDEBUG")
  endif()
else()
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions("-DNDEBUG")
  endif()
endif()

# Option to enable sanitizers
set(USE_SANITIZER "" CACHE STRING "Define the sanitizer used to build binaries and tests")

if (USE_SANITIZER)
  include(AddFlagUtils)

  if (USE_SANITIZER STREQUAL "Address")
    message(STATUS "Using Address Sanitizer, expect slow down in the analysis.")
    add_compiler_linker_flag(REQUIRED "FSANITIZE_ADDRESS" "-fsanitize=address")
    add_compiler_linker_flag(OPTIONAL "FSANITIZE_ADDRESS_USE_AFTER_SCOPE" "-fsanitize-address-use-after-scope")
  elseif (USE_SANITIZER STREQUAL "Undefined")
    message(STATUS "Using Undefined Behavior Sanitizer, expect slow down in the analysis.")
    add_compiler_linker_flag(REQUIRED "FSANITIZE_UNDEFINED" "-fsanitize=undefined")
    add_compiler_linker_flag(REQUIRED "FNO_SANITIZE_VPTR" "-fno-sanitize=vptr")
  elseif (USE_SANITIZER STREQUAL "Address;Undefined" OR
          USE_SANITIZER STREQUAL "Undefined;Address")
    message(STATUS "Using Address Sanitizer and Undefined Behavior Sanitizer, expect slow down in the analysis.")
    add_compiler_linker_flag(REQUIRED "FSANITIZE_ADDRESS" "-fsanitize=address")
    add_compiler_linker_flag(REQUIRED "FSANITIZE_UNDEFINED" "-fsanitize=undefined")
    add_compiler_linker_flag(REQUIRED "FNO_SANITIZE_VPTR" "-fno-sanitize=vptr")
  elseif (USE_SANITIZER MATCHES "Memory(WithOrigins)?")
    message(STATUS "Using Memory Sanitizer, expect slow down in the analysis.")
    add_compiler_linker_flag(REQUIRED "FSANITIZE_MEMORY" "-fsanitize=memory")

    if(USE_SANITIZER STREQUAL "MemoryWithOrigins")
      add_compiler_linker_flag(REQUIRED "FSANITIZE_MEMORY_TRACK_ORIGINS" "-fsanitize-memory-track-origins")
    endif()
  elseif (USE_SANITIZER STREQUAL "Thread")
    message(STATUS "Using Thread Sanitizer, expect slow down in the analysis.")
    add_compiler_linker_flag(REQUIRED "FSANITIZE_THREAD" "-fsanitize=thread")
  elseif (USE_SANITIZER STREQUAL "Leak")
    message(STATUS "Using Leak Sanitizer, expect slow down in the analysis.")
    add_compiler_linker_flag(REQUIRED "FSANITIZE_LEAK" "-fsanitize=leak")
  else()
    message(FATAL_ERROR "Unsupported value of USE_SANITIZER: ${USE_SANITIZER}")
  endif()

  # Flags useful for all sanitizers
  add_compiler_flag(OPTIONAL "FNO_OMIT_FRAME_POINTER" "-fno-omit-frame-pointer")
endif()
