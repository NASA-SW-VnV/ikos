# Dependencies to run the tests
add_dependencies(build-frontend-llvm-tests ikos-import)

# Try to find FileCheck
find_program(LLVM_FILE_CHECK
  CACHE NAMES FileCheck
  HINTS ${LLVM_TOOLS_BINARY_DIR}
  DOC "FileCheck executable")
if (NOT LLVM_FILE_CHECK)
  message(WARNING "Could NOT find FileCheck. Tests for llvm-to-ar are disabled.")
endif()

function(add_import_test test_name test_directory)
  if (NOT LLVM_FILE_CHECK)
    return() # Skip the test
  endif()

  add_test(NAME "import-${test_name}"
           WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${test_directory}"
           COMMAND runtest
             --ikos-import "$<TARGET_FILE:ikos-import>"
             --file-check "${LLVM_FILE_CHECK}")
endfunction()

add_import_test(no-optimization no_optimization)
add_import_test(basic-optimization basic_optimization)
add_import_test(aggressive-optimization aggressive_optimization)
