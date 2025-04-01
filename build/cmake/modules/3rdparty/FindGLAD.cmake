include(CPM)

set(package GLAD)
set(version 2.0.8)
string(TOLOWER ${package} basename)
file(TO_CMAKE_PATH  "${CPM_PRELOAD}/${basename}-${version}.zip" archive)

if (CPM_DOWNLOAD)
  CPMAddPackage(NAME ${package}
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG        v${version}
    OVERRIDE_FIND_PACKAGE
    EXCLUDE_FROM_ALL YES
    GIT_SHALLOW      ON
    SOURCE_SUBDIR cmake)
else()
  CPMAddPackage(NAME ${package}
    VERSION ${version}
    URL     ${archive}
    OVERRIDE_FIND_PACKAGE
    EXCLUDE_FROM_ALL YES
    SOURCE_SUBDIR cmake)
endif()
