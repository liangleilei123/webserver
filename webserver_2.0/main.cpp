#include <iostream>
#include "EventLoop.h"
#include "Server.h"


int main() {
    int port = 50000;
    EventLoop* loop = new EventLoop();      //第一步应该先把监听的fd加入到epoll的监听中
    Server *server = new Server(loop,port);
    server->start();

    return 0;
}

//#include <arpa/inet.h>
//#include <cstring>
//#include <cassert>
//#include <cstdio>
//#include <sys/socket.h>
//#include <unistd.h>
//#include <iostream>
//#include "Util.h"
//#include "httpParse.h"
//#include <fcntl.h>
//#include "Epoll.h"
//#include "Channel.h"
//
//
//
//int main() {
//    //1.绑定
//    int sockFd = socket(AF_INET, SOCK_STREAM, 0);
//    struct sockaddr_in serv_addr;
//    bzero(&serv_addr, sizeof serv_addr);
//    serv_addr.sin_family = AF_INET;
//    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);;
//    serv_addr.sin_port = htons(50000);
//    bind(sockFd, (sockaddr *) &serv_addr, sizeof serv_addr);
//    //2.监听
//    setSocketNonBlocking(sockFd);
//    listen(sockFd, 4096);
//    //3.连接
////    struct sockaddr_in clnt_addr;
////    socklen_t clnt_addr_len = sizeof clnt_addr;
////    bzero(&clnt_addr, sizeof clnt_addr);
//
//    //5.I/O多路复用
////    int epfd = epoll_create1(0);
////    errIf(epfd == -1, "epoll create error");
////    struct epoll_event events[1024], ev;
////    bzero(&events, sizeof(events));
//
//    Epoll *epoll = new Epoll();
//    Channel* connectChannel = new Channel(epoll,sockFd);
//    connectChannel->enableEvent();
//    while (true) {
//        std::vector<Channel *> vec_channel = epoll->poll();
//        int nFds = vec_channel.size();
//        for(int i = 0;i < nFds;++i){
//            int fd = vec_channel[i]->getFd();
//            if(fd == sockFd){           //说明有新的连接
//                int newConn = 0;
//                errIf(((newConn = socket_accept(sockFd)) == -1), "accept error");
//                setSocketNonBlocking(newConn);
//                Channel* newCh = new Channel(epoll,newConn);
//                newCh->enableEvent();
//            }
//            else if(vec_channel[i]->getReadyEvent() & EPOLLIN){
//                HttpConnect connect1(vec_channel[i]->getFd());
//                connect1.handleRead();
//            }else {
//                std::cout << " anything else" << std::endl;
//            }
//        }
//    }

//    Epoll *epoll = new Epoll();
//    epoll->epoll_add(sockFd, EPOLLIN | EPOLLET);
//    while (true) {
//        std::vector<epoll_event> vec_events = epoll->poll();
//        for (int i = 0; i < vec_events.size(); ++i) {
//            if (vec_events[i].data.fd == sockFd) {      //说明有新的连接
//                int newConn = 0;
//                errIf(((newConn = socket_accept(sockFd)) == -1), "accept error");
//                setSocketNonBlocking(newConn);
//                epoll->epoll_add(newConn, EPOLLIN | EPOLLET);
//            } else if (vec_events[i].events & EPOLLIN) {
//                HttpConnect connect1(vec_events[i].data.fd);
//                connect1.handleRead();
//            } else {
//                std::cout << " anything else" << std::endl;
//            }
//        }
//
//    }


//    int listenFd = accept(sockFd, (sockaddr *) &clnt_addr, &clnt_addr_len);
//    int clnt_sockFd = 0;
//    while((clnt_sockFd = accept(sockFd,(sockaddr*)&clnt_addr,&clnt_addr_len))>0){
//
//        printf("new client fd %d! IP: %s Port: %d\n", clnt_sockFd, inet_ntoa(clnt_addr.sin_addr), clnt_addr.sin_port);
//
//        bzero(&ev, sizeof(ev));
//        ev.data.fd = clnt_sockFd;
//        ev.events = EPOLLIN | EPOLLET;
//        setSocketNonBlocking(clnt_sockFd);
//        epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sockFd, &ev);
//
//
//        //4.通信
//        char buff[4096];
//        bzero(buff,sizeof buff);
//        ssize_t nread = read(clnt_sockFd,buff,4096);
//        std::cout <<"nread = " << nread <<std::endl;
//        HttpConnect connect(clnt_sockFd);
//        connect.handleRead();
//    }
//
//
//}

