//
// Created by 梁磊磊 on 2023/8/1.
//

#include <unistd.h>
#include <cstring>
#include <iostream>
#include "Epoll.h"
#include "Util.h"
#include "Channel.h"
#include "Timer.h"

const int EVENTSMAX = 1024;
const int EPOLLWAIT_TIME = 10000;

Epoll::Epoll()
        : epollFd_(epoll_create1(0)),
          events_(nullptr),
          timerManager_(new TimerManager()){
    events_ = new epoll_event[EVENTSMAX];
    errIf(epollFd_ == -1, "epoll create error");
    bzero(events_, sizeof(*events_) * EVENTSMAX);
}

Epoll::~Epoll() {
    if (epollFd_ != -1) {
        close(epollFd_);
        epollFd_ = -1;
    }
    delete[] events_;
}

void Epoll::epoll_add(Channel *channel,int timeout) {
    if(timeout > 0){
        addTimer(channel,timeout);
    }
    struct epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.data.ptr = channel;
    ev.events = channel->getEvents();

    if (!channel->getInEpoll()) {
        errIf(epoll_ctl(epollFd_, EPOLL_CTL_ADD, channel->getFd(), &ev) == -1, "epoll add channel error");
    } else {
        errIf(epoll_ctl(epollFd_, EPOLL_CTL_MOD, channel->getFd(), &ev) == -1, "epoll modify channel error");
    }
    channel->setInEpoll();

}

void Epoll::epoll_mod(Channel *channel,int timeout) {
    if(timeout > 0){
        addTimer(channel,timeout);
    }
    struct epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.data.ptr = channel;
    ev.events = channel->getEvents();
    errIf(epoll_ctl(epollFd_, EPOLL_CTL_MOD, channel->getFd(), &ev) == -1, "epoll modify event error");
}

void Epoll::epoll_del(Channel *channel) {
//    std::cout << "epoll delete :" << channel->getFd() << std::endl;
    if (channel->getInEpoll()) {
        errIf(epoll_ctl(epollFd_, EPOLL_CTL_DEL, channel->getFd(), nullptr) == -1, "epoll delete fd error");
        channel->setInEpoll(false);
    }

}

std::vector<Channel *> Epoll::poll() {
    std::vector<Channel *> activeEvents;
    int nFds = epoll_wait(epollFd_, events_, EVENTSMAX, EPOLLWAIT_TIME);
    errIf(nFds == -1, "Epoll.cpp line:57 poll: epoll wait error");
    for (int i = 0; i < nFds; ++i) {
        Channel *ch = (Channel *) events_[i].data.ptr;
        ch->setReadyEvent(events_[i].events);
        activeEvents.emplace_back(ch);
    }
    return activeEvents;
}

void Epoll::addTimer(Channel *channel, int timeout) {
    HttpConnect *httpConnect = channel->getHolder();
    if (httpConnect) {
        timerManager_->addTimer(httpConnect, timeout);
    } else {
        std::cout << "timer add fail" << std::endl;
    }
}

void Epoll::handleExpiredChannel() {
    timerManager_->handleExpiredEvent();
}
