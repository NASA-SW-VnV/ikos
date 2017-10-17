add_library(test-parsing-gv-init SHARED test-parsing-gv-init.cpp)

target_link_libraries(test-parsing-gv-init arbos-api)

install(TARGETS test-parsing-gv-init DESTINATION lib OPTIONAL)
