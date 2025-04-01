include(CPM)

set(package Boost)
set(version 1.87.0)
string(TOLOWER ${package} basename)
file(TO_CMAKE_PATH "${CPM_PRELOAD}/${basename}-${version}.zip" archive)

if (EMSCRIPTEN)
  set(THREADS_PREFER_PTHREAD_FLAG TRUE)
  find_package(Threads REQUIRED)

  set(BOOST_THREAD_THREADAPI "pthread" CACHE STRING "")

  add_compile_definitions(BOOST_HAS_PTHREADS)
  add_compile_options(-sUSE_PTHREADS)
  add_link_options(-pthread -sPTHREAD_POOL_SIZE=32)
endif(EMSCRIPTEN)

if (CPM_DOWNLOAD)
  CPMAddPackage(NAME ${package}
    URL     https://github.com/boostorg/boost/releases/download/boost-${version}/boost-${version}-cmake.tar.xz
    VERSION ${version}
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
