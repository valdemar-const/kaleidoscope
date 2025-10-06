find_package(Boost COMPONENTS uuid nowide unit_test_framework REQUIRED)
find_package(Compiler REQUIRED)

add_executable(test-context-alt)
target_sources(test-context-alt PRIVATE test_context_alt.cpp)
target_link_libraries(test-context-alt
  PRIVATE
    compiler::big_object
    compiler::definition_fix
  PRIVATE
    kaleidoscope::alt
  PRIVATE
    Boost::nowide
    Boost::unit_test_framework)

add_test(NAME test-context-alt COMMAND test-context-alt)
