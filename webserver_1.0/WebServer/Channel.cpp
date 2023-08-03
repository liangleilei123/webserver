//
// Created by 梁磊磊 on 2023/7/3.
//

#include <sys/epoll.h>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include "Channel.h"
#include "EventLoop.h"
#include "Util.h"

Channel::Channel(EventLoop* loop,int fd)
:loop_(loop),
fd_(fd),
inEpoll_(false),
events_(0),
readyEvent(0),
useThreadPool_(false){

}

Channel::~Channel() {
    if(fd_ != -1){
        close(-1);
    }
    fd_ = -1;
}

int Channel::getFd() const{
    return fd_;
}

uint32_t Channel::getEvents() const {
    return events_;
}

uint32_t Channel::getReadyEvents() const {
    return readyEvent;
}

bool Channel::getInEpoll() const {
    return inEpoll_;
}

void Channel::setInEpoll() {
    inEpoll_ = true;
}

void Channel::setReadyEvents(uint32_t event) {
    readyEvent = event;
}

void Channel::enableReading() {             //将这个channel本身加入epoll中
    events_ |= EPOLLIN | EPOLLPRI;
    loop_->updateChannel(this);
}

void Channel::setEvents(uint32_t event) {
    events_ = event;
}

//这个函数用多线程
void Channel::handleEvent() {

    if(readyEvent & (EPOLLIN | EPOLLPRI)){
        if(useThreadPool_){
            loop_->addThread(readCallBack);
        }
        else{
            readCallBack();
        }
    }
    if(readyEvent & (EPOLLOUT)){
        if(useThreadPool_){
            loop_->addThread(writeCallBack);
        }else{
            writeCallBack();
        }
    }
}

void Channel::setReadCallBack(Channel::callBack readCb) {
    readCallBack = readCb;
}

void Channel::setUseThreadPool(bool use) {
    useThreadPool_ = use;
}

void Channel::useET() {
    events_ |= EPOLLET;
    loop_->updateChannel(this);
}





