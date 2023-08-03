#include <iostream>
#include "EventLoop.h"
#include "Server.h"


int main() {
    int port = 8000;
    EventLoop* loop = new EventLoop(port);      //第一步应该先把监听的fd加入到epoll的监听中
    Server *server = new Server(loop);
    server->start();

    return 0;
}


//#include <iostream>
//#include <string>
//#include "WebServer/ThreadPool.h"
//
//void print(int a, double b, const char *c, std::string d){
//    std::cout << a << b << c << d << std::endl;
//}
//
//void test(){
//    std::cout << "hello" << std::endl;
//}
//
//int main(int argc, char const *argv[])
//{
//    ThreadPool *poll = new ThreadPool();
//    std::function<void()> func = std::bind(print, 1, 3.14, " hello", std::string(" world"));
//    poll->addTask(func);
//    func = test;
//    poll->addTask(func);
//    delete poll;
//    return 0;
//}
