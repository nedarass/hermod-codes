cmake_minimum_required(VERSION 3.10)

project(polaris VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(polaris include)
add_executable(polaris main.cpp src/config_manager.cpp src/udp_broadcasting.cpp src/tcp_server.cpp)
