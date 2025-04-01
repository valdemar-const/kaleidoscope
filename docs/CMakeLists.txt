cmake_minimum_required(VERSION 3.12...3.29)

project(${CMAKE_PROJECT_NAME}Docs VERSION ${CMAKE_PROJECT_VERSION})

# -- Doxygen & Sphinx support

find_package(Doxygen REQUIRED)
find_package(Python  COMPONENTS Interpreter REQUIRED)

# -- Setup sphinx

set(SPHINX_BUILD sphinx-build)

set(SPHINX_SOURCE_DIR ${PROJECT_SOURCE_DIR}/source)
set(SPHINX_BUILD_DIR  ${CMAKE_BINARY_DIR}/docs/sphinx-build)

# -- Setup doxygen

set(DOXYGEN_PROJECT         ${CMAKE_PROJECT_NAME})
set(DOXYGEN_PROJECT_VERSION ${CMAKE_PROJECT_VERSION})
set(DOXYGEN_INPUT           ${KaleidoscopeCompiler_SOURCE_DIR})
set(DOXYGEN_OUTPUT          ${CMAKE_BINARY_DIR}/docs/doxygen-build)

list(APPEND EXCLUDE_PATTERNS "*/tests/*" "*/src/*")

foreach(PATTERN IN LISTS EXCLUDE_PATTERNS)
  list(APPEND LIST_DOXYGEN_EXCLUDE_PATTERNS "\nEXCLUDE_PATTERNS      += ${PATTERN}")
endforeach()
string(REPLACE ";" "" DOXYGEN_EXCLUDE_PATTERNS ${LIST_DOXYGEN_EXCLUDE_PATTERNS})

# -- Configure

configure_file(config/doxyfile.in ${CMAKE_BINARY_DIR}/docs/config/doxyfile        @ONLY)
configure_file(config/conf.py.in  ${CMAKE_BINARY_DIR}/docs/config/conf.py  @ONLY)

set(SPHINX_CONF_DIR ${CMAKE_BINARY_DIR}/docs/config)

# -- Generate docs

add_custom_target(${PROJECT_NAME}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/docs/doxygen-build
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/docs/sphinx-build
  COMMAND Doxygen::doxygen ${CMAKE_BINARY_DIR}/docs/config/doxyfile
  COMMAND ${SPHINX_BUILD} -c ${SPHINX_CONF_DIR} ${SPHINX_SOURCE_DIR} ${SPHINX_BUILD_DIR})
