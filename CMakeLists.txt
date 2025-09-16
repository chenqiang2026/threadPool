# CMake entry point
cmake_minimum_required (VERSION 3.5)
project (threadpool)
 
## Add headers files
include_directories (
	./,
	./include
)
 
set(HEADERS include/threadpool_V1.h)
 
set(SOURCES src/threadpool_V1.cpp src/main.cpp)

SET(CMAKE_CXX_FLAGS -pthread)
 
add_compile_options(
	-std=c++14
#        -D_DEBUG
#        -g
)
# main.cpp
add_executable(main ${HEADERS} ${SOURCES})
target_link_libraries(main)

# 为不同构建类型设置不同输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/exec)
## set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/exec/debug)
## set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/exec/release)
# 确保输出目录存在
#file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
