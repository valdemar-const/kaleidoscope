find_package(Boost COMPONENTS uuid nowide unit_test_framework REQUIRED)
find_package(Compiler REQUIRED)

add_executable(test-context)
target_sources(test-context PRIVATE test_context.cpp)
target_link_libraries(test-context
  PRIVATE
    compiler::big_object
  PRIVATE
    kaleidoscope::alt
  PRIVATE
    Boost::nowide
    Boost::unit_test_framework)

add_test(NAME test-context COMMAND test-context)
