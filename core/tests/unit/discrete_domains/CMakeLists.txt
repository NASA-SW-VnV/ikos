add_executable(test_discrete_domain test_discrete_domain.cpp)
target_link_libraries(test_discrete_domain ${GMPXX_LIB} ${GMP_LIB})

install(TARGETS test_discrete_domain RUNTIME DESTINATION bin)
