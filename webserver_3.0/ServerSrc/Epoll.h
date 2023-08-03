//
// Created by 梁磊磊 on 2023/8/1.
//

#ifndef HTTPTEST_EPOLL_H
#define HTTPTEST_EPOLL_H


#include <cstdint>
#include <vector>
#include <sys/epoll.h>
#include "Timer.h"
#include "Macros.h"

class Channel;
class TimerManager;

class Epoll {
public:
    Epoll();
    ~Epoll();

    void epoll_add(Channel* channel,int timeout);
    void epoll_mod(Channel* channel,int timeout);
    void epoll_del(Channel* channel);
//    std::vector<epoll_event> poll(int timeout = -1);
    std::vector<Channel*> poll();

    void addTimer(Channel* channel,int timeout);
    void handleExpiredChannel();

private:
    DISALLOW_COPY_AND_MOVE(Epoll);

    int epollFd_;
    struct epoll_event *events_;
    TimerManager timerManager_;
};


#endif //HTTPTEST_EPOLL_H
