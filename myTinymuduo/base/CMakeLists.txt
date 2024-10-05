cmake_minimum_required(VERSION 3.10.0)
project(logger)

set(LIBRARY_OUTPUT_PATH ${lib_path})
include_directories(${resou})
aux_source_directory(${PROJECT_SOURCE_DIR} out)
add_library(log STATIC ${out})

#测试日志代码