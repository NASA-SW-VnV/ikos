add_executable(test_pta test_pta.cpp)
target_link_libraries(test_pta ${GMPXX_LIB} ${GMP_LIB})

install(TARGETS test_pta RUNTIME DESTINATION bin)
