add_library(test-api-factory SHARED test-api-factory.cpp)

target_link_libraries(test-api-factory arbos-api)

install(TARGETS test-api-factory DESTINATION lib OPTIONAL)
