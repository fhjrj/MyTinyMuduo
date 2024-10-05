cmake_minimum_required(VERSION 3.10.0)
project(Httpmodol)

set(LIBRARY_OUTPUT_PATH ${lib_path})
link_directories(${lib_path})
link_libraries(muduotext)#连接muduo静态库


include_directories(${resou})
aux_source_directory(${PROJECT_SOURCE_DIR} httpserver)
add_library(http STATIC ${httpserver})

set(HTTP httptext.cpp)
set(EXECUTABLE_OUTPUT_PATH ${outpath})
find_package(Threads)
link_directories(${lib_path})
link_libraries(http)
add_executable(HTTPSERVER ${HTTP})
target_link_libraries(HTTPSERVER ${CMAKE_THREAD_LIBS_INIT})


