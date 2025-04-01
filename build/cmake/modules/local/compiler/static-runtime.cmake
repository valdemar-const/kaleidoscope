# cmake/compiler/StaticRuntime.cmake

# Защита от двойного включения
if(TARGET static_runtime)
  return()
endif()

add_library(static_runtime INTERFACE)

if(MSVC)
  target_link_options(static_runtime INTERFACE "/MT$<$<CONFIG:Debug>:d>")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_link_options(static_runtime INTERFACE -static-libgcc -static-libstdc++)
else()
  message(WARNING "Static runtime not supported for ${CMAKE_CXX_COMPILER_ID}")
endif()

# Псевдоним для удобства
add_library(compiler::static_runtime ALIAS static_runtime)
