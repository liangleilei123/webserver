//
// Created by 梁磊磊 on 2023/8/1.
//

#include <iostream>
#include "EventLoop.h"
#include "Epoll.h"
#include "Channel.h"

EventLoop::EventLoop()
:epoll_(new Epoll()),
quit_(false){

}

EventLoop::~EventLoop() {
    delete epoll_;
}

void EventLoop::loop() {
    std::vector<Channel *> vec_channel;
    while (!quit_) {
        vec_channel.clear();
        vec_channel = epoll_->poll();
        for(auto& ch:vec_channel){
            ch->handleEvent();
        }
        epoll_->handleExpiredChannel();
    }
}

void EventLoop::addToEpoll(Channel* ch,int timeout) {           //使用这个函数之前要设置事件类型
    epoll_->epoll_add(ch,timeout);
}

void EventLoop::updateEpoll(Channel* ch,int timeout) {          //使用这个函数之前要设置事件类型
    epoll_->epoll_mod(ch,timeout);
}

void EventLoop::removeFromEpoll(Channel *ch) {
    epoll_->epoll_del(ch);
}
