//
// Created by 梁磊磊 on 2023/7/9.
//

#include <cstring>
#include <unistd.h>
#include "Connection.h"
#include "Util.h"


Connection::Connection(EventLoop* loop,int fd)
:loop_(loop),
connectFd_(fd),
connectChannel_(nullptr),
readBuf_(nullptr){
    connectChannel_ = new Channel(loop_,fd);
    std::function<void()> cb = std::bind(&Connection::echo,this);
    connectChannel_->useET();
    connectChannel_->setReadCallBack(cb);
    connectChannel_->enableReading();
    connectChannel_->setUseThreadPool(true);
    readBuf_ = new Buffer();
}
Connection::~Connection() {
    close(connectFd_);
    delete connectChannel_;
}

//void Connection::echo() {
//    char buffer[1024];
//    while(true){
//        bzero(&buffer,sizeof(buffer));
//        ssize_t bytes_read = read(connectFd_,buffer,sizeof(buffer));
//        if(bytes_read > 0){
//            printf("message from client fd %d: %s\n", connectFd_, buffer);
//            write(connectFd_, buffer, sizeof(buffer));
//        } else if(bytes_read == -1 && errno == EINTR){  //客户端正常中断、继续读取
//            printf("continue reading");
//            continue;
//        } else if(bytes_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))){//非阻塞IO，这个条件表示数据全部读取完毕
//            printf("finish reading once, errno: %d\n", errno);
//            break;
//        } else if(bytes_read == 0){  //EOF，客户端断开连接
//            printf("EOF, client fd %d disconnected\n", connectFd_);
////            close(connectFd_);
//            deleteConnectCB_(connectFd_);   //关闭socket会自动将文件描述符从epoll树上移除
//            break;
//        }
//    }
//}

void Connection::echo() {
    char buffer[1024];              //每次从缓冲区读取数据的大小，太大和太小都不合适
    while(true){
        bzero(&buffer,sizeof(buffer));
        ssize_t bytes_read = read(connectFd_,buffer,sizeof(buffer));
        if(bytes_read > 0){
            //printf("message from client fd %d: %s\n", connectFd_, buffer);
            readBuf_->append(buffer,bytes_read);
        } else if(bytes_read == -1 && errno == EINTR){  //客户端正常中断、继续读取
            printf("continue reading");
            continue;
        } else if(bytes_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))){//非阻塞IO，这个条件表示数据全部读取完毕
            printf("finish reading once...\n");
            printf("message from client fd %d: %s\n",connectFd_, readBuf_->c_str());
            send();
            readBuf_->clear();
            break;
        } else if(bytes_read == 0){  //EOF，客户端断开连接
            printf("EOF, client fd %d disconnected\n", connectFd_);
//            close(connectFd_);
            deleteConnectCB_(connectFd_);   //关闭socket会自动将文件描述符从epoll树上移除
            break;
        }
    }
}

void Connection::setDeleteConnectCallback(std::function<void(int)> cb) {
    deleteConnectCB_ = cb;
}

void Connection::send() {
    char buf[readBuf_->size()];
    strcpy(buf,readBuf_->c_str());
    int dataSize = readBuf_->size();
    int data_left = dataSize;
    while(data_left > 0){
        ssize_t bytes_write = write(connectFd_,buf + dataSize - data_left,data_left);
        if(bytes_write == -1 || errno == EAGAIN){
            break;
        }
        data_left -= bytes_write;
    }

}
