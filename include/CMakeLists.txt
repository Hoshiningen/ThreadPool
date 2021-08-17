add_library(threadpool-lib INTERFACE)

target_sources(threadpool-lib INTERFACE ThreadPool.h)
target_compile_features(threadpool-lib INTERFACE cxx_std_20)

set_target_properties(threadpool-lib PROPERTIES FOLDER ${PROJECT_NAME})
