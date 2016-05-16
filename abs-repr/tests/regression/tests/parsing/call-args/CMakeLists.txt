enable_testing()
add_test(NAME call-args-parsing
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
         COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/runtest)
