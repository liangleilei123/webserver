//
// Created by 梁磊磊 on 2023/7/2.
//

#include <cassert>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include "Epoll.h"
#include "Util.h"


//const int EVENTSNUM = 4096;
const int READ_BUFFER = 1024;

Epoll::Epoll()
        : epollFd_(epoll_create1(EPOLL_CLOEXEC)),
          events_(new epoll_event[EVENTSNUM]) {
    bzero(events_, sizeof(*events_) * EVENTSNUM);      //任何数据在使用之前要初始化
    assert(epollFd_ > 0);
}

Epoll::~Epoll() {
    if (epollFd_ != -1) {
        close(epollFd_);
        epollFd_ = -1;
    }
    delete[] events_;
}

//void Epoll::addFd(int fd, uint32_t option) {
//    struct epoll_event ev;
//    bzero(&ev, sizeof(ev));
//    ev.events = option;
//    ev.data.fd = fd;
//    errIf(epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add fd failure");
//}

void Epoll::addFd(Channel *_channel, uint32_t option) {
    int fd = _channel->getFd();
    struct epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.events = option;
    ev.data.ptr = _channel;
    errIf(epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add fd fail");
}

//返回当前的事件数组
//std::vector<epoll_event> Epoll::poll(int timeout) {
//    std::vector<epoll_event> activeEvents;
//    int nFds = epoll_wait(epollFd_, events_, EVENTSNUM, timeout);
//    for (int i = 0; i < nFds; ++i) {
//        activeEvents.push_back(events_[i]);
//    }
//    return activeEvents;
//}

std::vector<Channel *> Epoll::poll(int timeout) {
    std::vector<Channel *> activeEvents;
    int nFds = epoll_wait(epollFd_, events_, EVENTSNUM, timeout);
    errIf(nFds == -1, "epoll wait error");
    for (int i = 0; i < nFds; ++i) {
        Channel *ch = (Channel *) events_[i].data.ptr;
        ch->setReadyEvents(events_[i].events);
        activeEvents.push_back(ch);
    }
    return activeEvents;
}

//监听端口，有事件发生时找到socketFd处理
//void Epoll::eventsHandle(int listenFd) {
//    addFd(listenFd, EPOLLIN);
//    while (true) {
//        int nFds = epoll_wait(epollFd_, events_, EVENTSNUM, -1);       //这个函数会把有事件的文件描述符重排到前nFds
//        for (int i = 0; i < nFds; ++i) {        //遍历有事件发生的文件描述符
//            if (events_[i].data.fd == listenFd) {           //有新的客户端连接
//                int transFd = socket_accept(listenFd);
//                setSocketNonBlocking(transFd);          //设置非阻塞IO
//                addFd(transFd, EPOLLIN | EPOLLET);
//            } else if (events_[i].events & EPOLLIN) {       //发生事件的是客户端，并且是可读事件（EPOLLIN）
//                readHandle(&events_[i]);
//            } else {
//                printf("something else happened\n");
//            }
//        }
//    }
//    close(listenFd);
//}

void Epoll::updateChannel(Channel *channel) {
    int fd = channel->getFd();              //一个channel只维护一个文件描述符fd
    struct epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.data.ptr = channel;
    ev.events = channel->getEvents();
    if (!channel->getInEpoll()) {
        //errIf(epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, events_) == -1, "epoll add error");
        errIf(epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add error");
        channel->setInEpoll();
    } else {
        errIf(epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &ev) == -1, "epoll modify error");
    }

}

