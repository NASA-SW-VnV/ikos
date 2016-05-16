add_library(test-parsing-uid-internal_var SHARED test-parsing-uid-internal_var.cpp)

target_link_libraries(test-parsing-uid-internal_var arbos-api)

install(TARGETS test-parsing-uid-internal_var DESTINATION lib OPTIONAL)
