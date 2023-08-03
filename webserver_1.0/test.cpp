//
// Created by 梁磊磊 on 2023/7/11.
//
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <functional>
#include <arpa/inet.h>
#include <random>
#include "WebServer/Buffer.h"
#include "WebServer/ThreadPool.h"

using namespace std;


void oneClient(int msgs, int wait){

//1、创建套接字
    int fd = socket(AF_INET,SOCK_STREAM,0);
    if(fd == -1){
        perror("socket");
        return;
    }
    //2、建立连接
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = 8000;
    inet_pton(AF_INET,"127.0.0.1",&addr.sin_addr.s_addr);
    int ct = connect(fd,(struct sockaddr*)&addr,sizeof(addr));
    if(ct == -1){
        perror("connect");
        return ;
    }

    //3.和服务器通信
    Buffer *sendBuffer = new Buffer();
    Buffer *readBuffer = new Buffer();

    //
    int number = 0;
    default_random_engine e;    //生成随机的睡眠数
    uniform_int_distribution<unsigned > u(0,3);
    while(number < msgs){
        sleep(u(e));
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
        char buffer[1024];
        sprintf(buffer,"你好，服务器...%d\n",number++);
        sendBuffer->append(buffer,sizeof(buffer));
        ssize_t write_bytes = write(fd, sendBuffer->c_str(), sendBuffer->size());
        sendBuffer->clear();
        if(write_bytes == -1){
            printf("socket already disconnected, can't write any more!\n");
            break;
        }
        int already_read = 0;
        char buf[1024];    //这个buf大小无所谓
        while(true){
            bzero(&buf, sizeof(buf));
            ssize_t read_bytes = read(fd, buf, sizeof(buf));
            if(read_bytes > 0){
                readBuffer->append(buf, read_bytes);
                already_read += read_bytes;
            } else if(read_bytes == 0){         //EOF
                printf("server disconnected!\n");
                exit(EXIT_SUCCESS);
            }
            if(already_read >= sendBuffer->size()){
                printf("message from server: %s\n", readBuffer->c_str());
                break;
            }
        }
        readBuffer->clear();
    }
    close(fd);
}

int main(int argc, char *argv[]) {
    int threads = 5;
    int msgs = 10;
    int wait = 1;
    int o;
    const char *optstring = "t:m:w:";
    while ((o = getopt(argc, argv, optstring)) != -1) {
        switch (o) {
            case 't':
                threads = stoi(optarg);
                break;
            case 'm':
                msgs = stoi(optarg);
                break;
            case 'w':
                wait = stoi(optarg);
                break;
            case '?':
                printf("error optopt: %c\n", optopt);
                printf("error opterr: %d\n", opterr);
                break;
        }
    }

    ThreadPool *poll = new ThreadPool(threads);
    std::function<void()> func = std::bind(oneClient, msgs, wait);
    for(int i = 0; i < 10000; ++i){
        poll->addTask(func);
    }
    while(1);
    delete poll;
    return 0;
}