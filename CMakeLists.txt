cmake_minimum_required(VERSION 3.13)
project(GasStation)

set(CMAKE_CXX_STANDARD 17)

add_executable(GasStation main.cpp main.cpp resources.h)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(GasStation Threads::Threads)