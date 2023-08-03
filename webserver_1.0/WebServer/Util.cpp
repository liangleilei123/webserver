//
// Created by 梁磊磊 on 2023/7/5.
//

#include "Util.h"
#include <cstdio>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>


#define MAXBUFFSIZE 1024
#define READ_BUFFER 1024

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
//    addr.sin_port = ntohs((unsigned short)port);        //ntohs 将一个短整形从主机字节序转换到网络字节序
    addr.sin_port = port;
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
    printf("new client fd %d! IP: %s Port: %d\n", transFd, inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port));
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

void readHandle(struct epoll_event* events){
    //处理只读事件
    char buffer[READ_BUFFER];
    while(true){
        bzero(&buffer,sizeof(buffer));
        ssize_t bytes_read = read(events->data.fd,buffer,sizeof(buffer));
        if(bytes_read > 0){
            printf("message from client fd %d: %s\n", events->data.fd, buffer);
            write(events->data.fd, buffer, sizeof(buffer));
        } else if(bytes_read == -1 && errno == EINTR){  //客户端正常中断、继续读取
            printf("continue reading");
            continue;
        } else if(bytes_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))){//非阻塞IO，这个条件表示数据全部读取完毕
            printf("finish reading once, errno: %d\n", errno);
            break;
        } else if(bytes_read == 0){  //EOF，客户端断开连接
            printf("EOF, client fd %d disconnected\n", events->data.fd);
            close(events->data.fd);   //关闭socket会自动将文件描述符从epoll树上移除
            break;
        }
    }
}

//实现和客户端的通信

void communication(int listenFd){
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    int connectFd = accept(listenFd,(struct sockaddr*)&clientAddr,&clientLen);
    errIf(connectFd == -1,"accept fail");
    while(1){
        char buffer[MAXBUFFSIZE];                                   //定义缓冲区
        memset(buffer,0,sizeof(buffer));                    //清空缓冲区
        int read_len = read(connectFd,buffer,sizeof(buffer));
        if(read_len > 0){
            printf("客户端say: %s\n",buffer);
            write(connectFd,buffer,sizeof(buffer));        //发送数据
        }
        else if(read_len == 0) {
            std::cout << "客户端断开了连接..." << std::endl;

        }
        else{
            perror("read fail");
        }
    }

}