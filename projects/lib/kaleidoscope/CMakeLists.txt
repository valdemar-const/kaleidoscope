cmake_minimum_required(VERSION 3.12)

string(TOLOWER ${CMAKE_PROJECT_NAME} target)
add_library(${target} INTERFACE)
target_compile_features(${target} INTERFACE cxx_std_20)
target_include_directories(${target} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
