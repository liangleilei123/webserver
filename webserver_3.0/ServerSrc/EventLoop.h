//
// Created by 梁磊磊 on 2023/8/1.
//

#ifndef HTTPTEST_EVENTLOOP_H
#define HTTPTEST_EVENTLOOP_H

#include <memory>
#include <vector>
#include <functional>
#include "Macros.h"

class Epoll;
class Channel;

class EventLoop {
public:
    EventLoop();
    ~EventLoop();

    void loop();

    void addToEpoll(Channel* ch,int timeout = 0);
    void updateEpoll(Channel* ch,int timeout = 0);
    void removeFromEpoll(Channel* ch);



private:
    DISALLOW_COPY_AND_MOVE(EventLoop);
    std::unique_ptr<Epoll> epoll_;
    std::vector<std::function<void()>> pendingFunctors_;

};


#endif //HTTPTEST_EVENTLOOP_H
