if (${PROJECT_NAME}_WITH_TESTS AND NOT EMSCRIPTEN)
  enable_testing()
  add_subdirectory(${PROJECT_SOURCE_DIR}/tests)
endif()

add_subdirectory(${PROJECT_SOURCE_DIR}/projects/lib/kaleidoscope)
add_subdirectory(${PROJECT_SOURCE_DIR}/projects/lib/kaleidoscope-alt)
add_subdirectory(${PROJECT_SOURCE_DIR}/projects/bin/kalc)
add_subdirectory(${PROJECT_SOURCE_DIR}/docs)
