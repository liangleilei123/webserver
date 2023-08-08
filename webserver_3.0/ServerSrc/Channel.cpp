//
// Created by 梁磊磊 on 2023/8/1.
//

#include <unistd.h>
#include <iostream>
#include <sys/epoll.h>
#include "Channel.h"
#include "EventLoop.h"

Channel::Channel(EventLoop *loop, int fd)
        : exist_(false),
          loop_(loop),
          fd_(fd),
          events_(0),
          readyEvents_(0) ,
          holder_(nullptr){


}

Channel::~Channel() {
    close(fd_);
}

int Channel::getFd() {
    return fd_;
}

void Channel::setInEpoll(bool exist) {
    exist_ = exist;
}

bool Channel::getInEpoll() const {
    return exist_;
}

void Channel::setEvents(__uint32_t ev) {
    events_ = ev;
}

__uint32_t& Channel::getEvents() {
    return events_;
}

void Channel::setReadyEvent(__uint32_t ev) {
    readyEvents_ = ev;
}

__uint32_t Channel::getReadyEvent() const {
    return readyEvents_;
}

void Channel::setReadFunc(const std::function<void()> &readFunc) {
    readFunc_ = readFunc;
}

void Channel::handleReading() {
    if (readFunc_) {
        readFunc_();
    } else {
        std::cout << "Channel.cpp line:60 handleReading: readFunc not define" << std::endl;
    }
}

void Channel::setWriteFunc(const std::function<void()> &writeFunc) {
    writeFunc_ = writeFunc;
}

void Channel::handleWriting() {
    if (writeFunc_) {
        writeFunc_();
    } else {
        std::cout << "Channel line:73 handleWriting: writeFunc not define" << std::endl;
    }
}

void Channel::setConnectFunc(const std::function<void()> &connectFunc) {
    connectFunc_ = connectFunc;
}

void Channel::handleConnecting() {
    if (connectFunc_) {
        connectFunc_();
    }
//    else {
//        std::cout << "Channel line:86 handleConnecting: connectFunc not define" << std::endl;
//    }
}

void Channel::handleEvent() {
    events_ = 0;
    if((readyEvents_ & EPOLLHUP) && !(readyEvents_ & EPOLLIN)){
        events_ = 0;
        return;
    }
    if(readyEvents_ & EPOLLERR){
        //这里缺个错误处理
//        handleError();
        events_ = 0;
        return;
    }
    if(readyEvents_ &(EPOLLIN | EPOLLPRI | EPOLLHUP)){
        handleReading();
    }
    if(readyEvents_ &(EPOLLOUT)){
        handleWriting();
    }
    handleConnecting();
}

void Channel::enableEvent() {
    events_ |= EPOLLIN | EPOLLET;
//    events_ |= EPOLLIN ;
   loop_->addToEpoll(this);
}

void Channel::setHolder(HttpConnect* httpConnect) {
    holder_ = httpConnect;
}

HttpConnect *Channel::getHolder() {
    return holder_;
}

void Channel::setErrorFunc(const std::function<void()>& errorFunc) {
    errorFunc_ = errorFunc;
}

void Channel::handleError() {
    errorFunc_();
}
