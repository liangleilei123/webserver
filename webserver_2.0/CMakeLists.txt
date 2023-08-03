cmake_minimum_required(VERSION 3.16)
project(HttpTest)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(ServerSrc)



add_executable(main main.cpp)

target_link_libraries(main server)
target_include_directories(main PUBLIC ServerSrc)

