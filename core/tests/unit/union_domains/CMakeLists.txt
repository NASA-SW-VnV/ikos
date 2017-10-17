add_executable(test_union_domain test_union_domain.cpp)
target_link_libraries(test_union_domain ${GMPXX_LIB} ${GMP_LIB})

install(TARGETS test_union_domain RUNTIME DESTINATION bin)
