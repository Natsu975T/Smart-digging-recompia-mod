cmake_minimum_required(VERSION 3.20)
project(smart_digging_core C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(smart_digging_selection STATIC src/selection.c)
target_include_directories(smart_digging_selection PUBLIC src)

include(CTest)
if(BUILD_TESTING)
  add_executable(test_selection tests/test_selection.c)
  target_link_libraries(test_selection PRIVATE smart_digging_selection)
  add_test(NAME selection COMMAND test_selection)
endif()
