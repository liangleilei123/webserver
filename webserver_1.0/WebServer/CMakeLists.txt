set(SRCS
        EventLoop.cpp
        Epoll.cpp
        Channel.cpp
        HttpData.cpp
        Timer.cpp
        Server.cpp
        Util.cpp
        Connection.cpp
        Buffer.cpp
        ThreadPool.cpp
    )

add_library(webserver ${SRCS})      #将指定的源文件编译为目标文件

include_directories(${PROJECT_SOURCE_DIR}/base)
target_link_libraries(webserver libserver_base)
add_subdirectory(base)