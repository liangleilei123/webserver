//
// Created by 梁磊磊 on 2023/7/9.
//

#ifndef WEBS_CONNECTION_H
#define WEBS_CONNECTION_H

#include "Channel.h"
#include "Buffer.h"
#include "Macros.h"

class EventLoop;

class Connection {

public:
    Connection(EventLoop* loop,int fd);
    ~Connection();


    void echo();
    void setDeleteConnectCallback(std::function<void(int)>);
    void send();



private:
    DISALLOW_COPY_AND_MOVE(Connection);

    Channel* connectChannel_;
    EventLoop* loop_;
    int connectFd_;
    Buffer* readBuf_;
    std::function<void(int)> deleteConnectCB_;
};


#endif //WEBS_CONNECTION_H
