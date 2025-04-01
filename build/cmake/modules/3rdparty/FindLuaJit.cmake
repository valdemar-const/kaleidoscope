if (OFF)
  set(package LuaJit)
  set(version 2.1)
  string(TOLOWER ${package} basename)
  file(TO_CMAKE_PATH "${CPM_PRELOAD}/${basename}-${version}.zip" archive)

  CPMAddPackage(NAME ${package}
    VERSION    ${version}
    URL        ${archive}
    SOURCE_DIR "${CMAKE_BINARY_DIR}/_deps/luajit-build"
    OVERRIDE_FIND_PACKAGE
    EXCLUDE_FROM_ALL
    DOWNLOADONLY)

  # FIXME: msvc only(
  add_custom_command(
    OUTPUT            "${LuaJit_SOURCE_DIR}/src/lua51.lib"
    WORKING_DIRECTORY ${LuaJit_SOURCE_DIR}/src
    COMMAND           call "D:/Program Files/Microsoft Visual Studio/2022/VC/Auxiliary/Build/vcvars64.bat" & call "${LuaJit_SOURCE_DIR}/src/msvcbuild.bat" static)

  add_library(luajit STATIC IMPORTED)
  add_library(lua::lua ALIAS luajit)
  add_dependencies(luajit luajit_build)
  set_target_properties(luajit PROPERTIES IMPORTED_LOCATION "${LuaJit_SOURCE_DIR}/src/lua51.lib")
  target_link_libraries(luajit INTERFACE "${LuaJit_SOURCE_DIR}/src/lua51.lib")
  target_include_directories(luajit INTERFACE "${LuaJit_SOURCE_DIR}/src")
endif()
