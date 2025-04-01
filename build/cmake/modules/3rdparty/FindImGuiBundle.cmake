include(CPM)

find_package(GLAD    REQUIRED)
find_package(LunaSVG REQUIRED)

glad_add_library(glad STATIC REPRODUCIBLE LANGUAGE cpp API gl:core=3.3)
target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(glad::glad ALIAS glad)

set(package ImGuiBundle)
set(version 1.6.1)
set(basename imgui_bundle)
file(TO_CMAKE_PATH  "${CPM_PRELOAD}/${basename}-${version}.zip" archive)

if (CPM_DOWNLOAD)
  CPMAddPackage(NAME ${package}
    VERSION ${version}
    GIT_REPOSITORY https://github.com/pthom/imgui_bundle.git
    GIT_TAG        v${version}
    OVERRIDE_FIND_PACKAGE
    GIT_SHALLOW      ON
    EXCLUDE_FROM_ALL YES
    PATCH_COMMAND
       python -m patch -p1 ${CPM_PATCH_PREFIX}/${basename}-${version}/0000_fix_embed_wasm_data_support.patch
    && python -m patch -p1 ${CPM_PATCH_PREFIX}/${basename}-${version}/0001_fix_find_deps.patch)
else()
  CPMAddPackage(NAME ${package}
    VERSION ${version}
    URL     ${archive}
    OVERRIDE_FIND_PACKAGE
    EXCLUDE_FROM_ALL YES
    PATCH_COMMAND
       python -m patch -p1 ${CPM_PATCH_PREFIX}/${basename}-${version}/0000_fix_embed_wasm_data_support.patch
    && python -m patch -p1 ${CPM_PATCH_PREFIX}/${basename}-${version}/0001_fix_find_deps.patch)
endif()

target_link_libraries(imgui PUBLIC lunasvg glad)
target_include_directories(${basename} INTERFACE ${HELLOIMGUI_IMGUI_SOURCE_DIR}/.. ${lunasvg_SOURCE_DIR}/include)
