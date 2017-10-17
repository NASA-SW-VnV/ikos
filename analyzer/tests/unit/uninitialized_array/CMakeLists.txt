add_executable(test_uninitialized_array test_uninitialized_array.cpp)
target_link_libraries(test_uninitialized_array ${GMPXX_LIB} ${GMP_LIB} ${Boost_LIBRARIES})

install(TARGETS test_uninitialized_array RUNTIME DESTINATION bin)
