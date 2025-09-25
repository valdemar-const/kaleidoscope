find_package(Boost COMPONENTS nowide unit_test_framework REQUIRED)
find_package(Compiler REQUIRED)

add_executable(test-parser)
target_sources(test-parser PRIVATE test_parser.cpp)
target_link_libraries(test-parser
  PRIVATE
    compiler::big_object
  PRIVATE
    kaleidoscope
  PRIVATE
    Boost::nowide
    Boost::unit_test_framework)

add_test(NAME test-parser COMMAND test-parser)
