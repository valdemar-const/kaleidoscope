cmake_minimum_required(VERSION 3.12)

find_package(Boost COMPONENTS callable_traits uuid spirit fusion REQUIRED)
find_package(AnyAny REQUIRED)
find_package(Compiler REQUIRED)

string(TOLOWER ${CMAKE_PROJECT_NAME} target)
add_library(${target} INTERFACE)
target_compile_features(${target} INTERFACE cxx_std_20)
target_include_directories(${target} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${target} INTERFACE anyanylib Boost::callable_traits Boost::uuid Boost::spirit Boost::fusion compiler::demangle)

set(details KaleidoscopeDetails)
add_library(${details} INTERFACE)
target_sources(${details}
  INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/support/from_span_char.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/support/from_span_uint8_t.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/support/from_string_view.cpp)

target_link_libraries(${details} INTERFACE ${target})

target_link_libraries(${target} INTERFACE ${details})
