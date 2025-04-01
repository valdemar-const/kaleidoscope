cmake_minimum_required(VERSION 3.12)

project(KaleidoscopeCompiler VERSION 1.0 LANGUAGES CXX)

set(target ${PROJECT_NAME})
add_executable(${target})
target_sources(${target} PRIVATE src/main.cpp)
target_link_libraries(${target} PRIVATE kaleidoscope)
set_target_properties(${target}
  PROPERTIES
    OUTPUT_NAME              kalc
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
