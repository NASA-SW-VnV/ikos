find_package(APRON)
if (APRON_FOUND)
  include_directories(${APRON_INCLUDE_DIR})

  add_executable(test_apron test_apron.cpp)
  target_link_libraries(test_apron ${GMPXX_LIB} ${GMP_LIB} ${APRON_LIBRARIES})

  install(TARGETS test_apron RUNTIME DESTINATION bin)
endif()
