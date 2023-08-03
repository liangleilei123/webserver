//
// Created by 梁磊磊 on 2023/7/1.
//

#include "EventLoop.h"

EventLoop::EventLoop(int port) :
port_(port),
epoll_(new Epoll()),
threadPool_(nullptr){
    threadPool_ = new ThreadPool();
}
EventLoop::~EventLoop(){
    delete epoll_;
    delete threadPool_;
}

void EventLoop::loop() {
    while(!quit_){
        std::vector<Channel*> chs;
        chs = epoll_->poll();           //获取事件队列
        for(auto it = chs.begin();it != chs.end();++it){
            (*it)->handleEvent();
        }
    }
}

void EventLoop::updateChannel(Channel *channel) {
    epoll_->updateChannel(channel);
}

int EventLoop::getPort() {
    return port_;
}

void EventLoop::addThread(std::function<void()> func) {
    threadPool_->addTask(func);
}


