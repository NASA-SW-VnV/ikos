add_library(check-srcloc SHARED check-srcloc.cpp)

target_link_libraries(check-srcloc arbos-api)

install(TARGETS check-srcloc DESTINATION lib OPTIONAL)
