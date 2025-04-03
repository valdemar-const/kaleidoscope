cmake_minimum_required(VERSION 3.12)

find_package(Boost COMPONENTS spirit fusion REQUIRED)

string(TOLOWER ${CMAKE_PROJECT_NAME} target)
add_library(${target} INTERFACE)
target_compile_features(${target} INTERFACE cxx_std_20)
target_include_directories(${target} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${target} INTERFACE Boost::spirit Boost::fusion)

set(details KaleidoscopeDetails)
add_library(${details} INTERFACE)
target_sources(${details}
  INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/support/from_span_char.cpp)
target_link_libraries(${details} INTERFACE ${target})

target_link_libraries(${target} INTERFACE ${details})
