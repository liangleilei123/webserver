 //
// Created by 梁磊磊 on 2023/8/1.
//

#include <iostream>
#include <thread>
#include <cstring>
#include <netinet/in.h>
#include "Server.h"
#include "Util.h"
#include "Channel.h"
#include "EventLoop.h"
#include "httpParse.h"
#include "ThreadPool.h"

Server::Server(EventLoop *loop,int port)
:mainReactor_(loop),
port_(port){
    listenFd_ = socket_bind_listen(port);
    setSocketNonBlocking(listenFd_);
    acceptChannel_ = new Channel(mainReactor_,listenFd_);
    acceptChannel_->setEvents(EPOLLIN | EPOLLET);
    acceptChannel_->setReadFunc(std::bind(&Server::newConnection,this));
    mainReactor_->addToEpoll(acceptChannel_);



}

Server::~Server() {
    delete acceptChannel_;
}

void Server::start() {
    mainReactor_->loop();
}

void Server::newConnection() {
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    socklen_t client_addr_len = sizeof(client_addr);
    int newConn = 0;
    while ((newConn = accept(listenFd_,
                             (struct sockaddr *)&client_addr,&client_addr_len)) > 0) {

        int random = newConn % subReactors_.size();

        HttpConnect *newHttpConn = new HttpConnect(mainReactor_, newConn);
        setSocketNonBlocking(newConn);
        connections_[newConn] = newHttpConn;
        newHttpConn->newEvent();
    }
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


