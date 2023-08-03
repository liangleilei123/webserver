set(SRCS
        HttpConnect.cpp
        Channel.cpp
        Server.cpp
        EventLoop.cpp
        ThreadPool.cpp
        Epoll.cpp
        Timer.cpp
        Util.cpp)

add_library(server ${SRCS})

include_directories(${PROJECT_SOURCE_DIR}/base)
target_link_libraries(server libserver_base)
add_subdirectory(base)