//
// Created by 梁磊磊 on 2023/7/29.
//

#ifndef HTTPTEST_UTIL_H
#define HTTPTEST_UTIL_H

#include <sys/socket.h>
#include <sys/epoll.h>
#include <string>



void errIf(bool condition, const char* errMsg);
int socket_bind_listen(int port);
int setSocketNonBlocking(int socketFd);
ssize_t readn(int fd, void *buff, size_t n);
ssize_t readn(int fd, std::string &inBuffer, bool &zero);
ssize_t readn(int fd, std::string &inBuffer);
ssize_t writen(int fd, void *buff, size_t n);
ssize_t writen(int fd, std::string &sbuff);

void shutDownWR(int fd);
void handle_for_sigpipe();


#endif //HTTPTEST_UTIL_H
