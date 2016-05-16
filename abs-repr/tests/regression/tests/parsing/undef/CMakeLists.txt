enable_testing()
add_test(NAME undef
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
         COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/runtest)
