include(CPM)

set(package GLFW)
set(version 3.4)
string(TOLOWER ${package} basename)

if (EMSCRIPTEN)
  set(basename emscripten-${basename})
  set(url     https://github.com/pongasoft/emscripten-glfw.git)
  set(tag     v${version}.0.20250209)
else()
  set(url     https://github.com/glfw/glfw.git)
  set(tag     ${version})
endif()

file(TO_CMAKE_PATH "${CPM_PRELOAD}/${basename}-${version}.zip" archive)

if (CPM_DOWNLOAD)
  CPMAddPackage(NAME ${package}
    VERSION ${version}
    GIT_REPOSITORY ${url}
    GIT_TAG        ${tag}
    OVERRIDE_FIND_PACKAGE
    GIT_SHALLOW      ON
    EXCLUDE_FROM_ALL YES)
else()
  CPMAddPackage(NAME ${package}
    VERSION ${version}
    URL     ${archive}
    OVERRIDE_FIND_PACKAGE
    EXCLUDE_FROM_ALL YES)
endif()

if (EMSCRIPTEN)
  if (NOT TARGET glfw::glfw)
    set(glfw3_DIR ${GLFW_SOURCE_DIR})
    add_library(glfw       ALIAS glfw3)
    add_library(glfw::glfw ALIAS glfw3)
    message(STATUS "GLFW_SOURCE_DIR: ${glfw3_DIR}")
  endif()
else()
  if (NOT TARGET glfw::glfw)
    set(glfw3_DIR ${GLFW_SOURCE_DIR} CACHE FILEPATH "" FORCE)
    add_library(glfw::glfw ALIAS glfw)
  endif()
endif()
