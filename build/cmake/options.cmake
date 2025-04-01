# CPM specific options
option(USE_CPM      "Use cmake package manager for 3rdparty dependencies." ON)
option(CPM_DOWNLOAD "Allow CPM download 3rdparty dependencies."            OFF)

# Toplevel Project specific
option(${PROJECT_NAME}_WITH_TESTS "Build tests" ON)

set(CPM_PATCH_PREFIX ${PROJECT_SOURCE_DIR}/build/patches)

set(CMAKE_CXX_STANDARD          20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS        NO)
