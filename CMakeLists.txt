cmake_minimum_required(VERSION 2.8)
project(prj)

if (EXISTS "${CMAKE_SOURCE_DIR}/minisat")
  add_subdirectory(minisat)
else()
  message (FATAL_ERROR "MiniSat not found. Use `git clone https://github.com/agurfinkel/minisat` to check it out.")
endif()

find_package (Threads)

## Specific configuration start here
## Project specific configurations go here

# enable c++11 support
set (CMAKE_CXX_FLAGS "-std=c++11 -Wall ${CMAKE_CXX_FLAGS}")

# expose minisat header files
include_directories(${CMAKE_SOURCE_DIR}/minisat)

# create the main executable
## add additional .cpp files if needed
add_executable(prj prj.cpp)
# link MiniSAT libraries
target_link_libraries(prj minisat-lib-static)

# link pthread libraries
target_link_libraries(prj ${CMAKE_THREAD_LIBS_INIT})

# create the executable for tests
#add_executable(test test.cpp)
# link MiniSAT libraries
#target_link_libraries(test minisat-lib-static)
