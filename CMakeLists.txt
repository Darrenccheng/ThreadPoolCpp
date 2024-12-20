cmake_minimum_required(VERSION 3.17)
project(mythreadPoolCPP)

set(CMAKE_CXX_STANDARD 11)
file(GLOB SRC ${PROJECT_SOURCE_DIR}/*.cpp)
find_package(Threads REQUIRED)
add_executable(mythreadPoolCPP ${SRC})
target_link_libraries(mythreadPoolCPP Threads::Threads)