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
#include <csignal>


#define MAXBUFFSIZE 1024
#define READ_BUFFER 1024
const int MAX_BUFF = 1024;


// 错误处理函数
void errIf(bool condition, const char *errMsg) {
    if (condition) {
        perror(errMsg);
        exit(-1);
    }
}


//socket的绑定和监听
//成功：返回文件描述符
//失败：返回-1
int socket_bind_listen(int port) {
    if (port < 0 || port > 65535) {
        return -1;
    }

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        return -1;
    }

    //设置套接字可以重新绑定同一地址(SO_REUSEADDR)，以消除bind时"Address already in use"错误
    int optVal = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optVal,
                   sizeof(optVal)) == -1) {
        close(listen_fd);
        return -1;
    }


    //绑定端口和ip地址
    struct sockaddr_in addr;
    memset(&addr,0,sizeof addr);
//    bzero(&addr, sizeof(addr));          //初始化
    addr.sin_family = AF_INET;
//    addr.sin_port = ntohs(port);        //ntohs 将一个短整形从主机字节序转换到网络字节序
    addr.sin_port = htons(port);
//    addr.sin_port = port;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);            //htonl 将整形主机字节序转换到网络字节序。
//    addr.sin_addr.s_addr =INADDR_ANY;            //htonl 将整形主机字节序转换到网络字节序。
    /* ip地址设置为INADDR_ANY，INADDR_ANY 是一个特殊的 IPv4 地址常量，它的值是 0。
     * 当套接字绑定到 INADDR_ANY 地址时，表示该套接字将接受任意网络接口的连接请求。
     * 这通常用于服务器程序，以便在多个网络接口上监听连接请求。
     */
    if (bind(listen_fd, (struct sockaddr *) &addr,
             sizeof(addr)) == -1) {
        close(listen_fd);
        return -1;
    }

    // 监听
    // SOMAXCONN，表示系统允许排队的最大连接数量。默认是4096,排队的socket是等待连接状态，即使对端发送完数据，这个排队的连接也不会取消
    if (listen(listen_fd, SOMAXCONN) == -1) {
        close(listen_fd);
        return -1;
    }
    return listen_fd;

}


//把socket连接设置为非阻塞类型
int setSocketNonBlocking(int socketFd) {
    int flag = fcntl(socketFd, F_GETFL, 0);           //fcntl 函数的 F_GETFL 命令获取套接字的标志位。
    errIf(flag == -1, "set socketFd nonblocking failure");       //获取失败，flag = -1
    flag |= O_NONBLOCK;                                     //将获取到的标志位添加上 O_NONBLOCK 标志，以设置套接字为非阻塞模式。
    if (fcntl(socketFd, F_SETFL, flag) == -1) {        //调用 fcntl 函数的 F_SETFL 命令将修改后的标志位设置回套接字中。
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
    LOG << "--------reading from: "<<fd;
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
            zero = true;
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

/*    ssize_t nread = 0;
    ssize_t readSum = 0;

    while (true) {
//        bzero(buff, sizeof buff);
        char buff[MAX_BUFF];
        memset(buff,0,sizeof buff);
        nread = read(fd, buff, MAX_BUFF);
        if (nread > 0) {
//            printf("before inBuffer.size() = %d\n", inBuffer.size());
//            printf("nread = %d\n", nread);
            readSum += nread;
//            buff += nread;
            inBuffer += std::string(buff, buff + nread);
//            printf("after inBuffer.size() = %d\n", inBuffer.size());
        } else if (nread < 0) {
            if (errno == EINTR)
                continue;
            else if (errno == EAGAIN) {             //没有数据可读
//                std::cout << " errno = EAGAIN" << std::endl;
                return readSum;
            } else {
                perror("read error");
                return -1;
            }
        } else if (nread == 0) {                    //对端已关闭
//            printf("readsum = %d\n", readSum);
            zero = true;
            break;
        }

    }

    return readSum; */
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
    if(fd < 0){
        return -1;
    }
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
    if(fd < 0){
        return -1;
    }
    LOG << "------writing to: " <<fd;
    size_t nleft = sbuff.size();
    ssize_t nwritten = 0;
    ssize_t writeSum = 0;
    const char *ptr = sbuff.c_str();
    while (nleft > 0) {
        if ((nwritten = write(fd, ptr, nleft)) <= 0) {
            if (nwritten < 0) {
                LOG << " file error ";
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

void handle_for_sigpipe() {
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    if (sigaction(SIGPIPE, &sa, NULL)) return;
}
