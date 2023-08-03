//
// Created by 梁磊磊 on 2023/7/5.
//

#ifndef WEBS_SERVER_H
#define WEBS_SERVER_H

#include <sys/socket.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <map>
#include "Epoll.h"
#include "EventLoop.h"
#include "Connection.h"
#include "Macros.h"

class Connection;

//主要实现一个服务器端的socket
class Server {

public:
    //构造函数完成文件描述符初始化、绑定端口，
    Server(EventLoop* loop);
    ~Server();

    void start();

    void newConnectHandle();
    void deleteConnection(int fd);

private:
    DISALLOW_COPY_AND_MOVE(Server);

    int listenFd_;        //监听的文件描述符
    EventLoop* mainReactor_;
    std::vector<EventLoop*> subReactors_;
    Channel* acceptChannel_;
    std::map<int ,Connection*> connections_;

    ThreadPool *threadPool_;

};


#endif //WEBS_SERVER_H
