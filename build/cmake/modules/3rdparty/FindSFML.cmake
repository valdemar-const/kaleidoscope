include(CPM)

set(package SFML)
set(version 3.0.0)
string(TOLOWER ${package} basename)
file(TO_CMAKE_PATH "${CPM_PRELOAD}/${basename}-${version}.zip" archive)

if (CPM_DOWNLOAD)
  CPMAddPackage(NAME ${package}
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG        ${version}
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
