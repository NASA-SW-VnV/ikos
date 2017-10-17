add_library(test-parsing-stack-allocate SHARED test-stack-allocate-parsing.cpp)

target_link_libraries(test-parsing-stack-allocate arbos-api)

install(TARGETS test-parsing-stack-allocate DESTINATION lib OPTIONAL)
