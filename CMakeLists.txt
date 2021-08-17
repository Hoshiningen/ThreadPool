cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)
project(ThreadPool CXX)

OPTION(BUILD_BENCHMARKS "Build benchmarks" OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set directory paths
set(CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Pull down all the linking cmake scripts
include(${CMAKE_DIR}/FetchCMakeUtilities.cmake)
FetchCMakeUtilities()

add_executable(threadpool-app source/main.cpp)

target_link_libraries(threadpool-app PRIVATE threadpool-lib)
target_include_directories(threadpool-app PRIVATE include)

set_target_properties(threadpool-app PROPERTIES FOLDER ${PROJECT_NAME})
set_target_properties(threadpool-app PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)

add_subdirectory(include)

if (BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()