//
// Created by 梁磊磊 on 2023/8/1.
//

#ifndef HTTPTEST_SERVER_H
#define HTTPTEST_SERVER_H

#include <map>
#include <vector>

class HttpConnect;
class EventLoop;
class Channel;
class ThreadPool;

class Server {
public:
    Server(EventLoop* loop,int port);
    ~Server();

    void start();

    void newConnection();
    void deleteConnection(int fd);

private:
    EventLoop *mainReactor_;
    int listenFd_;
    int port_;

    Channel *acceptChannel_;
    std::map<int,HttpConnect*> connections_;
    std::vector<EventLoop*> subReactors_;
    ThreadPool* thPool_;


};


#endif //HTTPTEST_SERVER_H
