include(CPM)

set(package Freetype)
set(version 2.13.3)
set(tag VER-2-13-3)
string(TOLOWER ${package} basename)
file(TO_CMAKE_PATH  "${CPM_PRELOAD}/${basename}-${version}.zip" archive)

if (CPM_DOWNLOAD)
  CPMAddPackage(NAME ${package}
    VERSION ${version}
    GIT_REPOSITORY https://github.com/freetype/freetype.git
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
