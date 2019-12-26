cmake_minimum_required(VERSION 3.10)
project(http)

set(CMAKE_SYSTEM_NAME Linux)
set(CROSS_COMPILE )
set(CMAKE_C_COMPILER ${CROSS_COMPILE}gcc)
set(CMAKE_C_FLAGS "-Wall -g")

set(TARGET_NAME httpdemo)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/libs/libevent/include)

link_directories("${PROJECT_SOURCE_DIR}/libs")
link_directories("${PROJECT_SOURCE_DIR}/libs/libevent/lib")

set(TARGET_LIBS pthread)

file(GLOB_RECURSE INCLUDES "include/*.h")

file(GLOB_RECURSE SOURCES "src/*.c")

aux_source_directory(./src DIR_SRCS)

message(STATUS "DIR_SRCS:${DIR_SRCS}")
message(STATUS "SOURCES:${SOURCES}\n INCLUDES:${INCLUDES}")

add_executable(${TARGET_NAME} ${SOURCES})

target_link_libraries(${TARGET_NAME} ${TARGET_LIBS})