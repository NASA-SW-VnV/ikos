add_executable(test_var_packing_dbm test_var_packing_dbm.cpp)
target_link_libraries(test_var_packing_dbm ${GMPXX_LIB} ${GMP_LIB})

install(TARGETS test_var_packing_dbm RUNTIME DESTINATION bin)
