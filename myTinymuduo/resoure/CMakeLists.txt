cmake_minimum_required(VERSION 3.10.0)
project(muduo)

set(LIBRARY_OUTPUT_PATH ${lib_path})
aux_source_directory(${PROJECT_SOURCE_DIR} out)
add_library(muduotext STATIC ${out})
