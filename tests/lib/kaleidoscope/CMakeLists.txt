find_package(Boost COMPONENTS uuid nowide unit_test_framework callable_traits REQUIRED)
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
    Boost::unit_test_framework
    Boost::callable_traits)

add_executable(test-context)
target_sources(test-context PRIVATE test_context.cpp)
target_link_libraries(test-context
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
    Boost::unit_test_framework
    Boost::callable_traits)

add_test(NAME test-parser COMMAND test-parser)
add_test(NAME test-context COMMAND test-context)
