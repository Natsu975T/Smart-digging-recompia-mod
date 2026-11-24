cmake_minimum_required(VERSION 3.20)
project(smart_digging LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(smart_digging_selection STATIC src/selection.c)
target_include_directories(smart_digging_selection PUBLIC src)
set_target_properties(smart_digging_selection PROPERTIES POSITION_INDEPENDENT_CODE ON)

include(CTest)
if(BUILD_TESTING)
  add_executable(test_selection tests/test_selection.c)
  target_link_libraries(test_selection PRIVATE smart_digging_selection)
  add_test(NAME selection COMMAND test_selection)
endif()

set(MODTOPIA_SDK_DIR "" CACHE PATH "Path to Modtopia SDK include directory")
if(MODTOPIA_SDK_DIR)
  add_library(smart_digging SHARED src/mod.cpp)
  target_include_directories(smart_digging PRIVATE src "${MODTOPIA_SDK_DIR}")
  target_link_libraries(smart_digging PRIVATE smart_digging_selection)
  set_target_properties(smart_digging PROPERTIES
    PREFIX ""
    OUTPUT_NAME "smart_digging"
    SUFFIX ".so"
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES)
endif()
