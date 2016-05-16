enable_testing()
add_test(NAME bitwise-condition-parsing-select
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
         COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/runtest)
