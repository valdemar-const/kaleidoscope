include(CPM)

set(package EnTT)
set(version 3.14.0)
string(TOLOWER ${package} basename)
file(TO_CMAKE_PATH  "${CPM_PRELOAD}/${basename}-${version}.zip" archive)

if (CPM_DOWNLOAD)
  CPMAddPackage(NAME ${package}
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG        v${version}
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
