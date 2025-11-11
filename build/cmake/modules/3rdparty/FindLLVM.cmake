include(CPM)

set(package LLVM)
set(version 21.1.5)
string(TOLOWER ${package} basename)
file(TO_CMAKE_PATH  "${CPM_PRELOAD}/${basename}-${version}.zip" archive)

if (CPM_DOWNLOAD)
  CPMAddPackage(NAME ${package}
    VERSION        ${version}
    GIT_REPOSITORY https://github.com/llvm/llvm-project.git
    GIT_TAG        llvmorg-${version}
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
