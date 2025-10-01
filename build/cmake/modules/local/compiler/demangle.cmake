# Защита от двойного включения
if(TARGET demangle)
  return()
endif()

add_library(demangle INTERFACE)

target_include_directories(demangle INTERFACE ${CMAKE_CURRENT_LIST_DIR}/demangle/include)
add_library(compiler::demangle ALIAS demangle)
