include(CPM)

set(package LunaSVG)
set(version 3.0.1)
string(TOLOWER ${package} basename)
file(TO_CMAKE_PATH  "${CPM_PRELOAD}/${basename}-${version}.zip" archive)

if (CPM_DOWNLOAD)
  CPMAddPackage(NAME ${package}
    GIT_REPOSITORY https://github.com/sammycage/lunasvg.git
    GIT_TAG        v${version}
    OVERRIDE_FIND_PACKAGE
    GIT_SHALLOW      ON
    EXCLUDE_FROM_ALL YES
    PATCH_COMMAND python -m patch -p1 ${CPM_PATCH_PREFIX}/${basename}-${version}/0000_fix_find_deps.patch)
else()
  CPMAddPackage(NAME ${package}
    VERSION ${version}
    URL     ${archive}
    OVERRIDE_FIND_PACKAGE
    EXCLUDE_FROM_ALL YES
    PATCH_COMMAND python -m patch -p1 ${CPM_PATCH_PREFIX}/${basename}-${version}/0000_fix_find_deps.patch)
endif()
