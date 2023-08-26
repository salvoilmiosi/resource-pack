cmake_minimum_required(VERSION 3.13.0)
project(resource_pack VERSION 0.1.0)

find_package(BZip2 REQUIRED)

add_library(resource_pack_interface INTERFACE)
target_include_directories(resource_pack_interface INTERFACE include)
target_link_libraries(resource_pack_interface INTERFACE ${BZIP2_LIBRARIES})

add_executable(resource_pack src/resource_pack.cpp)
target_link_libraries(resource_pack PRIVATE resource_pack_interface)

add_executable(resource_unpack src/resource_unpack.cpp)
target_link_libraries(resource_unpack PRIVATE resource_pack_interface)

add_library(resource_load STATIC src/resource_load.cpp)
target_link_libraries(resource_load PUBLIC resource_pack_interface)