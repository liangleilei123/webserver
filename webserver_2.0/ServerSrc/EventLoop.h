//
// Created by 梁磊磊 on 2023/8/1.
//

#ifndef HTTPTEST_EVENTLOOP_H
#define HTTPTEST_EVENTLOOP_H

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
    Epoll *epoll_;
    bool quit_;
};


#endif //HTTPTEST_EVENTLOOP_H
