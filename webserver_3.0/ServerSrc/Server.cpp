//
// Created by 梁磊磊 on 2023/8/1.
//

#include <iostream>
#include <thread>
#include "Server.h"
#include "Util.h"
#include "Channel.h"
#include "EventLoop.h"
#include "HttpConnect.h"
#include "ThreadPool.h"

Server::Server(EventLoop *loop,int port)
:mainReactor_(loop),
port_(port){
    listenFd_ = socket_bind_listen(port);
    setSocketNonBlocking(listenFd_);
    acceptChannel_ = std::make_unique<Channel>(mainReactor_,listenFd_);
//    acceptChannel_->setEvents(EPOLLIN | EPOLLET);
    acceptChannel_->setReadFunc(std::bind(&Server::newConnection,this));
//    mainReactor_->addToEpoll(acceptChannel_);
    acceptChannel_->enableEvent();

    int size = std::thread::hardware_concurrency();
//    thPool_ = new ThreadPool(size);
    thPool_ = std::make_unique<ThreadPool>(size);
    for(int i = 0;i < size;++i){
//        subReactors_.push_back(new EventLoop());
        std::unique_ptr<EventLoop> sub_reactor = std::make_unique<EventLoop>();
        subReactors_.push_back(std::move(sub_reactor));
    }

    for(int i = 0;i < size;++i){
        std::function<void()> sub_loop = std::bind(&EventLoop::loop,subReactors_[i].get());
        thPool_->addTask(sub_loop);
    }

}

Server::~Server() {
//    delete acceptChannel_;
}

void Server::start() {
    mainReactor_->loop();
}

void Server::newConnection() {
    int newConn = 0;
    errIf(((newConn = socket_accept(listenFd_)) == -1), "accept error");
    setSocketNonBlocking(newConn);
    int random = newConn % subReactors_.size();
    HttpConnect* newHttpConn = new HttpConnect(subReactors_[random].get(),newConn);
    newHttpConn->getChannel()->setHolder(newHttpConn);
    newHttpConn->setDeleteHttpConnFunc(std::bind(&Server::deleteConnection,this,std::placeholders::_1));
    connections_[newConn] = newHttpConn;
    newHttpConn->newEvent();
}

void Server::deleteConnection(int fd) {
    if(fd > 0){
        auto it = connections_.find(fd);
        if(it != connections_.end()){
            HttpConnect* conn = connections_[fd];
            connections_.erase(fd);
            delete conn;
        }
    }

}


