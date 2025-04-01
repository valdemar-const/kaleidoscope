# cmake/compiler/BigObject.cmake

if(TARGET big_object)
    return()
endif()

add_library(big_object INTERFACE)

if(MSVC)
    target_compile_options(big_object INTERFACE /bigobj /Gy)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  check_cxx_compiler_flag("-Wa,-mbig-obj" HAS_BIG_OBJ)  # Проверка поддержки флага
  if(HAS_BIG_OBJ)
    target_compile_options(big_object INTERFACE -Wa,-mbig-obj)
  else()
    message(WARNING "Compiler does not support -Wa,-mbig-obj")
  endif()
endif()

add_library(compiler::big_object ALIAS big_object)
