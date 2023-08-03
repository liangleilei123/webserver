//
// Created by 梁磊磊 on 2023/7/5.
//

#include <iostream>
#include <unistd.h>
#include "Server.h"
#include "Util.h"


Server::Server(EventLoop* loop)
:mainReactor_(loop),
listenFd_(socket_bind_listen(loop->getPort())),
acceptChannel_(nullptr)
{
    acceptChannel_ = new Channel(mainReactor_,listenFd_);
    std::function<void()> cb = std::bind(&Server::newConnectHandle,this);
    acceptChannel_->setReadCallBack(cb);
    acceptChannel_->enableReading();

    //创建副反应堆
    int size = std::thread::hardware_concurrency();
    threadPool_ = new ThreadPool(size);
//    std::cout<< size << "个 subReactor 被创建"  <<std::endl;
    for(int i = 0;i < size;++i){        //创建副反应堆
        subReactors_.emplace_back(new EventLoop(loop->getPort()));
    }


}

Server::~Server(){


    close(listenFd_);       //关闭监听
    delete acceptChannel_;

}

//完成通信
void Server::start() {
    for(int i = 0;i < subReactors_.size();++i){
        std::function<void()> sub_loop = std::bind(&EventLoop::loop,subReactors_[i]);
        threadPool_->addTask(sub_loop);         //一个线程绑定一个副反应堆
    }
    mainReactor_->loop();
}

void Server::newConnectHandle() {
    int acceptFd = socket_accept(listenFd_);
    int random = acceptFd % subReactors_.size();
    std::cout << acceptFd << " 号客户端链接被分配给 " << random << " 号 subReactor" << std::endl;
    setSocketNonBlocking(acceptFd);             //设置为非阻塞模式
    Connection* newConnection = new Connection(subReactors_[random],acceptFd);     //分配给副反应堆
    std::function<void(int)> cb = std::bind(&Server::deleteConnection,this,std::placeholders::_1);
    newConnection->setDeleteConnectCallback(cb);
    connections_[acceptFd] = newConnection;              //加入map

}


void Server::deleteConnection(int fd) {
    if( fd != -1){
        auto it = connections_.find(fd);
        if(it != connections_.end()){
            Connection* conn = connections_[fd];
            connections_.erase(fd);
            delete conn;
        }
    }
}


//void Server::deleteConnection(int fd) {
//    Connection* conn = connections[fd];
//    connections.erase(fd);
//    delete conn;
//}
