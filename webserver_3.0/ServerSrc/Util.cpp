//
// Created by 梁磊磊 on 2023/7/5.
//

#include "Util.h"
#include "base/Logging.h"
#include <cstdio>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include <errno.h>


#define MAXBUFFSIZE 1024
#define READ_BUFFER 1024
const int MAX_BUFF = 4096;


// 错误处理函数
void errIf(bool condition, const char* errMsg){
    if(condition){
        perror(errMsg);
        exit(-1);
    }
}


//socket的绑定和监听
//成功：返回文件描述符
//失败：返回-1
int socket_bind_listen(int port){
    errIf(port < 0 || port > 65535,"port error");

    int listen_fd = socket(AF_INET,SOCK_STREAM,0);
    errIf(listen_fd == -1,"socket create fail");

    //设置套接字可以重新绑定同一地址(SO_REUSEADDR)，以消除bind时"Address already in use"错误
    int optVal = 1;
    errIf(setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&optVal,sizeof(optVal)) == -1,
          "setSockOpt fail");

    //绑定端口和ip地址
    struct sockaddr_in addr;
    bzero(&addr,sizeof(addr));          //初始化
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(port);        //ntohs 将一个短整形从主机字节序转换到网络字节序
//    addr.sin_port = port;
    addr.sin_addr.s_addr =htonl(INADDR_ANY);            //htonl 将整形主机字节序转换到网络字节序。
    /* ip地址设置为INADDR_ANY，INADDR_ANY 是一个特殊的 IPv4 地址常量，它的值是 0。
     * 当套接字绑定到 INADDR_ANY 地址时，表示该套接字将接受任意网络接口的连接请求。
     * 这通常用于服务器程序，以便在多个网络接口上监听连接请求。
     */
    errIf(bind(listen_fd,(struct sockaddr*)&addr,sizeof(addr)) == -1,"bind fail");

    // 监听
    // SOMAXCONN，表示系统允许排队的最大连接数量。默认是4096
    errIf(listen(listen_fd,SOMAXCONN) == -1,"listen error");
    return listen_fd;

}

int socket_accept(int listenFd){
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int transFd = accept(listenFd, (struct sockaddr *) &client_addr, &client_addr_len);
    if (transFd == -1) {
        perror("socket accept error");
        exit(-1);
    }
//    printf("new client fd %d! IP: %s Port: %d\n", transFd, inet_ntoa(client_addr.sin_addr),
//           ntohs(client_addr.sin_port));
//    printf("new client fd %d! IP: %s Port: %d\n", transFd, inet_ntoa(client_addr.sin_addr),
//           client_addr.sin_port);
    LOG << "New connection from " << inet_ntoa(client_addr.sin_addr) << ":"
        << ntohs(client_addr.sin_port);

    return transFd;
}

//把socket连接设置为非阻塞类型
int setSocketNonBlocking(int socketFd){
    int flag = fcntl(socketFd,F_GETFL,0);           //fcntl 函数的 F_GETFL 命令获取套接字的标志位。
    errIf(flag == -1,"set socketFd nonblocking failure");       //获取失败，flag = -1
    flag |= O_NONBLOCK;                                     //将获取到的标志位添加上 O_NONBLOCK 标志，以设置套接字为非阻塞模式。
    if(fcntl(socketFd,F_SETFL,flag) == -1){        //调用 fcntl 函数的 F_SETFL 命令将修改后的标志位设置回套接字中。
        return -1;
    }           //设置失败返回-1，成功返回0
    return 0;
}

ssize_t readn(int fd, void *buff, size_t n) {
    size_t nleft = n;
    ssize_t nread = 0;
    ssize_t readSum = 0;
    char *ptr = (char *) buff;
    while (nleft > 0) {
        if ((nread = read(fd, ptr, nleft)) < 0) {
            if (errno == EINTR)
                nread = 0;
            else if (errno == EAGAIN) {
                return readSum;
            } else {
                return -1;
            }
        } else if (nread == 0)
            break;
        readSum += nread;
        nleft -= nread;
        ptr += nread;
    }
    return readSum;
}

ssize_t readn(int fd, std::string &inBuffer, bool &zero) {
//    std::cout<<"reading..."<<std::endl;

    ssize_t nread = 0;
    ssize_t readSum = 0;
    char buff[MAX_BUFF];

    while (true) {
        bzero(buff,sizeof buff);
        nread = read(fd,buff,MAX_BUFF);
        if(nread > 0){
//            printf("before inBuffer.size() = %d\n", inBuffer.size());
//            printf("nread = %d\n", nread);
            readSum += nread;
//            buff += nread;
            inBuffer += std::string(buff, buff + nread);
//            printf("after inBuffer.size() = %d\n", inBuffer.size());
        }
        else if (nread < 0) {
            if ( errno == EINTR)
                continue;
            else if (errno == EAGAIN) {
//                std::cout << " errno = EAGAIN" << std::endl;
                return readSum;
            } else {
                perror("read error");
                return -1;
            }
        } else if (nread == 0) {
//            printf("readsum = %d\n", readSum);
            zero = true;
            break;
        }

    }

    return readSum;
}

ssize_t readn(int fd, std::string &inBuffer) {
    ssize_t nread = 0;
    ssize_t readSum = 0;
    while (true) {
        char buff[MAX_BUFF];
        if ((nread = read(fd, buff, MAX_BUFF)) < 0) {
            if (errno == EINTR)
                continue;
            else if (errno == EAGAIN) {
                return readSum;
            } else {
                perror("read error");
                return -1;
            }
        } else if (nread == 0) {
            // printf("redsum = %d\n", readSum);
            break;
        }
        // printf("before inBuffer.size() = %d\n", inBuffer.size());
        // printf("nread = %d\n", nread);
        readSum += nread;
        // buff += nread;
        inBuffer += std::string(buff, buff + nread);
        // printf("after inBuffer.size() = %d\n", inBuffer.size());
    }
    return readSum;
}

ssize_t writen(int fd, void *buff, size_t n) {
    size_t nleft = n;
    ssize_t nwritten = 0;
    ssize_t writeSum = 0;
    char *ptr = (char *) buff;
    while (nleft > 0) {
        if ((nwritten = write(fd, ptr, nleft)) <= 0) {
            if (nwritten < 0) {
                if (errno == EINTR) {
                    nwritten = 0;
                    continue;
                } else if (errno == EAGAIN) {
                    return writeSum;
                } else
                    return -1;
            }
        }
        writeSum += nwritten;
        nleft -= nwritten;
        ptr += nwritten;
    }
    return writeSum;
}

ssize_t writen(int fd, std::string &sbuff) {
    size_t nleft = sbuff.size();
    ssize_t nwritten = 0;
    ssize_t writeSum = 0;
    const char *ptr = sbuff.c_str();
    while (nleft > 0) {
        if ((nwritten = write(fd, ptr, nleft)) <= 0) {
            if (nwritten < 0) {
                if (errno == EINTR) {
                    nwritten = 0;
                    continue;
                } else if (errno == EAGAIN)
                    break;
                else
                    return -1;
            }
        }
        writeSum += nwritten;
        nleft -= nwritten;
        ptr += nwritten;
    }
    if (writeSum == static_cast<int>(sbuff.size()))
        sbuff.clear();
    else
        sbuff = sbuff.substr(writeSum);
    return writeSum;
}

void shutDownWR(int fd) {
    shutdown(fd, SHUT_WR);
    // printf("shutdown\n");
}
