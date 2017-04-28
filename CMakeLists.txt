# CMakeLists.txt for event package. It creates a library with dictionary and a main program
cmake_minimum_required(VERSION 3.4 FATAL_ERROR)
project(AnalysisChain)
#---Compiler options control
set (CMAKE_CXX_FLAGS "-g -w -std=c++11")

# You need to tell CMake where to find the ROOT installation.
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
#---Locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
find_package(ROOT REQUIRED COMPONENTS)
#---Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#---Define VERBOSE runtime --- USER DEFINITION
option(VERBOSE "Verbose print mode" OFF)
if(VERBOSE)
  add_definitions(-DVerboseMode)
endif(VERBOSE)

#---Create a shared library with generated dictionary
add_library(GSFilter SHARED GSFilter.cpp)
target_link_libraries(GSFilter ${ROOT_LIBRARIES})

add_library(Decode SHARED Decode.cpp)
target_link_libraries(Decode GSFilter ${ROOT_LIBRARIES})

add_library(Event SHARED Event.cpp)
target_link_libraries(Event ${ROOT_LIBRARIES})

#---Create a main program using the library
add_executable(AnalysisChain AnalysisChain.cpp)
target_link_libraries(AnalysisChain Decode Event)

#---Copy data file over the build directory
set(Example_DATA
  example.bin
)


foreach(_data ${Example_DATA})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_data}
    ${PROJECT_BINARY_DIR}/${_data}
  )
endforeach()
