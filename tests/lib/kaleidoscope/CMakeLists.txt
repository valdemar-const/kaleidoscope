find_package(Boost COMPONENTS uuid nowide unit_test_framework REQUIRED)
find_package(Compiler REQUIRED)
find_package(AnyAny   REQUIRED)
find_package(EnTT     REQUIRED)
find_package(GSL      REQUIRED)

add_executable(test-parser)
target_sources(test-parser PRIVATE test_parser.cpp)
target_link_libraries(test-parser
  PRIVATE
    compiler::big_object
  PRIVATE
    kaleidoscope
  PRIVATE
    EnTT::EnTT
    anyanylib
  PRIVATE
    Boost::nowide
    Boost::uuid
    Boost::unit_test_framework)

add_test(NAME test-parser COMMAND test-parser)
