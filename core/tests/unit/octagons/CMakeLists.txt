add_executable(test_octagon_domain test_octagon_domain.cpp)
target_link_libraries(test_octagon_domain ${GMPXX_LIB} ${GMP_LIB})

install(TARGETS test_octagon_domain RUNTIME DESTINATION bin)
