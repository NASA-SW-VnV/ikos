add_library(test-parsing-exit-nodes SHARED test-parsing-exit-nodes.cpp)

target_link_libraries(test-parsing-exit-nodes arbos-api)

install(TARGETS test-parsing-exit-nodes DESTINATION lib OPTIONAL)
