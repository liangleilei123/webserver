//
// Created by 梁磊磊 on 2023/7/2.
//

#ifndef WEBS_EPOLL_H
#define WEBS_EPOLL_H

#include <vector>
#include <sys/epoll.h>
#include <memory>
#include "Channel.h"
#include "Macros.h"


const int EVENTSNUM = 4096;

//io复用的机制，实现高效的事件分发，提高网络应用的性能
class Epoll {
public:

    Epoll();
    ~Epoll();


    //void eventsHandle(int listenFd);
    void addFd(Channel*,uint32_t);
    //std::vector<epoll_event> poll(int time = -1);
    std::vector<Channel*> poll(int timeout = -1);
    void updateChannel(Channel*);

private:
    DISALLOW_COPY_AND_MOVE(Epoll);

    static const int MAXFDS = 100000;       //最大的文件描述符的数量
    int epollFd_;                           //epoll文件描述符
    struct epoll_event *events_;




};


#endif //WEBS_EPOLL_H
