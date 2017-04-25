# CMakeLists.txt for event package. It creates a library with dictionary and a main program
cmake_minimum_required(VERSION 3.4 FATAL_ERROR)
project(AnalysisChain)
set (CMAKE_CXX_FLAGS "-g -w -std=c++11")

# You need to tell CMake where to find the ROOT installation. This can be done in a number of ways:
#   - ROOT built with CMake. Add in CMAKE_PREFIX_PATH the installation prefix for ROOT
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

#---Locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
find_package(ROOT REQUIRED COMPONENTS)

#---Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#---Define DEBUG compilation
add_definitions(-DDEBUG)

#---Create a shared library with generated dictionary
add_library(DRS4Decode DRS4Decode.cpp)
target_link_libraries(DRS4Decode ${ROOT_LIBRARIES})

#---Create a main program using the library
add_executable(AnalysisChain main.cpp)
target_link_libraries(AnalysisChain DRS4Decode)
