cmake_minimum_required(VERSION 2.5)
project(mymuduo)
set(CMAKE_CXX_STANDARD 14)

set(loger ${PROJECT_SOURCE_DIR}/base)
set(resou ${PROJECT_SOURCE_DIR}/resoure)
set(lib_path ${PROJECT_SOURCE_DIR}/lib)#超过add_sub无法识别lib_path
set(outpath ${PROJECT_SOURCE_DIR}/out)
set(http ${PROJECT_SOURCE_DIR}/Http)

add_subdirectory(${PROJECT_SOURCE_DIR}/resoure)
add_subdirectory(${PROJECT_SOURCE_DIR}/base/text)
add_subdirectory(${PROJECT_SOURCE_DIR}/base)
add_subdirectory(${PROJECT_SOURCE_DIR}/Http)

set(CMAKE_CXX_FLAGS "-fPIC")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/out)




#测试MUDUO模块
set(demo ./text.cpp)
link_directories(${lib_path})
include_directories(${resou})#Tcpserver
link_libraries(muduotext)
find_package(Threads)
add_executable(a.out ${demo})
target_link_libraries(a.out ${CMAKE_THREAD_LIBS_INIT})