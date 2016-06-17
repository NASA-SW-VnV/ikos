add_executable(test_muaz test_muaz.cpp)
target_link_libraries (test_muaz ${ARBOS_LIB} ${GMPXX_LIB} ${GMP_LIB} ${Boost_LIBRARIES} ${RT_LIB} pthread)

install(TARGETS test_muaz RUNTIME DESTINATION bin)
