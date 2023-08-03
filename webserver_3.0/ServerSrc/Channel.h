//
// Created by 梁磊磊 on 2023/8/1.
//

#ifndef HTTPTEST_CHANNEL_H
#define HTTPTEST_CHANNEL_H


#include <cstdio>
#include <functional>
#include "Macros.h"

class EventLoop;
class HttpConnect;

class Channel {
public:
    Channel(EventLoop *loop,int fd);
    ~Channel();

    int getFd();
    void setInEpoll(bool exist = true);
    bool getInEpoll() const;

    void setEvents(__uint32_t ev);
    __uint32_t& getEvents();

    void setReadyEvent(__uint32_t ev);
    __uint32_t getReadyEvent() const;

    void setReadFunc(const std::function<void()> &readFunc);
    void handleReading();

    void setWriteFunc(const std::function<void()> &writeFunc);
    void handleWriting();

    void setConnectFunc(const std::function<void()> &connectFunc);
    void handleConnecting();

    void handleEvent();

    void enableEvent();

    void setHolder(HttpConnect* httpConnect);
    HttpConnect* getHolder();


private:
    DISALLOW_COPY_AND_MOVE(Channel);

    EventLoop *loop_;
    int fd_;
    HttpConnect* holder_;

    __uint32_t events_;                 //Channel维护的fd的事件类型
    __uint32_t readyEvents_;            //要执行的事件类型
    bool exist_;

    std::function<void()> readFunc_;
    std::function<void()> writeFunc_;
    std::function<void()> connectFunc_;

};


#endif //HTTPTEST_CHANNEL_H
