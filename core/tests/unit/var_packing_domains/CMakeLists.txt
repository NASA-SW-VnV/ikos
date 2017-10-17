add_executable(test_var_packing_domain test_var_packing_domain.cpp)
target_link_libraries(test_var_packing_domain ${GMPXX_LIB} ${GMP_LIB})

install(TARGETS test_var_packing_domain RUNTIME DESTINATION bin)
