# Защита от двойного включения
if(TARGET fix_definition)
  return()
endif()

add_library(fix_definition INTERFACE)

target_include_directories(fix_definition INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
add_library(compiler::definition_fix ALIAS fix_definition)
