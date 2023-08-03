cmake_minimum_required(VERSION 3.16)
project(webS)

set(CMAKE_CXX_STANDARD 14)

# 用于将WebServer目录添加到当前CMakeLists.txt所在目录的子目录中，并执行WebServer目录下的CMakeLists.txt文件。
add_subdirectory(WebServer)



add_executable(main main.cpp)               # 把 main.cpp 链接到一个名为 main 的可执行文件中，以便运行程序
target_link_libraries(main webserver)       # 在链接可执行文件 main 时，将名为 webserver 的库文件链接到可执行文件中。
target_include_directories(main PUBLIC WebServer)       #在main 中添加WebServer，作用是在当前目录下的文件使用WebServer中的文件时，#include不用再写相对路径，只用写文件名就行

target_link_libraries(main pthread)

add_executable(test test.cpp)
target_link_libraries(test webserver)       # 在链接可执行文件 main 时，将名为 webserver 的库文件链接到可执行文件中。
target_link_libraries(test pthread)