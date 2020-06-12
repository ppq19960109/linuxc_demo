cmake_minimum_required(VERSION 3.10)
project(http)

set(CMAKE_SYSTEM_NAME Linux)
set(CROSS_COMPILE )
set(CMAKE_C_COMPILER ${CROSS_COMPILE}gcc)
set(CMAKE_C_FLAGS "-Wall -g")

set(TARGET_NAME httpdemo)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/libs/libevent/include)
include_directories(${PROJECT_SOURCE_DIR}/libs/sqlite/include)
include_directories(${PROJECT_SOURCE_DIR}/libs/qrencode/include)

link_directories("${PROJECT_SOURCE_DIR}/libs")
link_directories("${PROJECT_SOURCE_DIR}/libs/libevent/lib")
link_directories("${PROJECT_SOURCE_DIR}/libs/sqlite/lib")
#link_directories("${PROJECT_SOURCE_DIR}/libs/qrencode/lib")

set(TARGET_LIBS -Wl,--start-group -Wl,-Bstatic -levent_core -levent  -levent_pthreads  -levent_extra -levent_openssl  -lsqlite3 
    -Wl,-Bdynamic -ldl -lm -lpthread -Wl,--end-group)

#message(STATUS "TARGET_LIBS:${TARGET_LIBS}\n")


file(GLOB_RECURSE INCLUDES "include/*.h")

file(GLOB_RECURSE SOURCES "src/*.c")

aux_source_directory(./src DIR_SRCS)

#message(STATUS "INCLUDES:${INCLUDES}\n")
#message(STATUS "SOURCES:${SOURCES}\n")
#message(STATUS "DIR_SRCS:${DIR_SRCS}\n")

file(GLOB_RECURSE QRCODE_SOURCE "qrcode/*.c")
file(GLOB_RECURSE QRCODE_INCLUDE "eveent_test/*.h")
add_library(eventtest ${QRCODE_SOURCE} ${QRCODE_INCLUDE})

add_executable(${TARGET_NAME} ${SOURCES})

target_link_libraries(${TARGET_NAME} ${TARGET_LIBS})