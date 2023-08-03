//
// Created by 梁磊磊 on 2023/7/5.
//

#ifndef WEBS_UTIL_H
#define WEBS_UTIL_H

#include <sys/socket.h>
#include <sys/epoll.h>


//socket的绑定和监听
//成功：返回文件描述符
//失败：返回-1
void errIf(bool condition, const char* errMsg);
int socket_bind_listen(int port);
int socket_accept(int listenFd);
int setSocketNonBlocking(int socketFd);
void readHandle(struct epoll_event* events );
//测试的代码
void communication(int connectFd);


#endif //WEBS_UTIL_H
