add_executable(threadpool-bench benchmarks.cpp)

target_link_libraries(threadpool-bench PRIVATE threadpool-lib)
target_include_directories(threadpool-bench PRIVATE ../include)

set_target_properties(threadpool-bench PROPERTIES FOLDER ${PROJECT_NAME})
set_target_properties(threadpool-bench PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)

include(${cmake_utility_SOURCE_DIR}/cmake/google/LinkGBenchmark.cmake)
LinkGBenchmark(threadpool-bench PRIVATE v1.5.5)
