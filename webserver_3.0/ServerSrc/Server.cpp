//
// Created by 梁磊磊 on 2023/8/1.
//

#include <iostream>
#include <thread>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "Server.h"
#include "Util.h"
#include "Channel.h"
#include "EventLoop.h"
#include "HttpConnect.h"
#include "ThreadPool.h"
#include "base/Logging.h"

Server::Server(EventLoop *loop,int port)
:mainReactor_(loop),
port_(port){
    listenFd_ = socket_bind_listen(port);
    errIf(listenFd_ == -1,"socket bind or listen error");
    setSocketNonBlocking(listenFd_);
    acceptChannel_ = std::make_unique<Channel>(mainReactor_,listenFd_);
    acceptChannel_->setReadFunc(std::bind(&Server::newConnection,this));
    acceptChannel_->enableEvent();

}

Server::~Server() {

}

void Server::start() {
    mainReactor_->loop();
}

void Server::newConnection() {
    LOG << "--------------------------------------------------------------------------------";
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    socklen_t client_addr_len = sizeof(client_addr);
    int newConn = 0;
    while ((newConn = accept(listenFd_, (struct sockaddr *)&client_addr,
                             &client_addr_len)) > 0){

        HttpConnect* newHttpConn = new HttpConnect(mainReactor_,newConn);
        setSocketNonBlocking(newConn);
        newHttpConn->setDeleteHttpConnFunc(std::bind(&Server::deleteConnection,this,std::placeholders::_1));
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







