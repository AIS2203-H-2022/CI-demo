cmake_minimum_required(VERSION 3.15)
project(live_demo)

set(CMAKE_CXX_STANDARD 17)

add_executable(live_demo main.cpp)
if (UNIX)
    target_link_libraries(live_demo PRIVATE pthread tbb)
endif()
