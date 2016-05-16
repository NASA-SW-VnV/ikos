add_executable(test_dbm test_dbm.cpp)
target_link_libraries(test_dbm ${GMPXX_LIB} ${GMP_LIB})

install(TARGETS test_dbm RUNTIME DESTINATION bin)
