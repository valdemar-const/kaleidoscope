cmake_minimum_required(VERSION 3.12)

find_package(Boost COMPONENTS uuid spirit fusion REQUIRED)
find_package(EnTT REQUIRED)
find_package(AnyAny REQUIRED)

string(TOLOWER "${CMAKE_PROJECT_NAME}alt" target)
add_library(${target} INTERFACE)
target_compile_features(${target} INTERFACE cxx_std_20)
target_include_directories(${target} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${target}
  INTERFACE
    Boost::uuid
    Boost::spirit
    Boost::fusion
  INTERFACE
    EnTT::EnTT
    anyanylib)
add_library(kaleidoscope::alt ALIAS ${target})
