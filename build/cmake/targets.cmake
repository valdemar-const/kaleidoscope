if (${PROJECT_NAME}_WITH_TESTS AND NOT EMSCRIPTEN)
  include(CTest)
endif()

add_subdirectory(${PROJECT_SOURCE_DIR}/projects/lib/kaleidoscope)
add_subdirectory(${PROJECT_SOURCE_DIR}/projects/bin/kalc)
add_subdirectory(${PROJECT_SOURCE_DIR}/docs)
