cmake_minimum_required(VERSION 3.10)
project(robot_ble_communicator)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

add_compile_options(-Wall -Wextra -O2)

include_directories(
    ${CMAKE_SOURCE_DIR}/inc
    ../vom/src/model/graph
    ../vsys/inc
    ../redisdb/inc
)

find_package(glog REQUIRED)
find_package(gflags REQUIRED)

file(GLOB RBC_BLE_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")

# 라이브러리 이름 통일!
add_library(rbc_ble STATIC ${RBC_BLE_SOURCES})
target_include_directories(rbc_ble PUBLIC ${CMAKE_SOURCE_DIR}/inc)
target_link_libraries(rbc_ble bluetooth pthread gflags glog::glog)

add_executable(rbc_test test/main.cpp)
target_link_libraries(rbc_test
    rbc_ble
    ${CMAKE_CURRENT_SOURCE_DIR}/../vom/lib/libEreon_ObjectModel.a
    ${CMAKE_CURRENT_SOURCE_DIR}/../vsys/lib/libvsys.a
    dbus-1
    bluetooth
    pthread
    gflags
    glog::glog
)

install(TARGETS rbc_test RUNTIME DESTINATION bin)
install(TARGETS rbc_ble ARCHIVE DESTINATION lib)
