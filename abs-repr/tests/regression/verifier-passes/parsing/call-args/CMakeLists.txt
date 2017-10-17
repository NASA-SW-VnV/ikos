add_library(test-parsing-call-args SHARED test-parsing-call-args.cpp)

target_link_libraries(test-parsing-call-args arbos-api)

install(TARGETS test-parsing-call-args DESTINATION lib OPTIONAL)
