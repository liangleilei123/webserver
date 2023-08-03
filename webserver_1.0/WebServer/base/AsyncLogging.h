//
// Created by 梁磊磊 on 2023/7/18.
//

#ifndef HTTPSERVER_ASYNCLOGGING_H
#define HTTPSERVER_ASYNCLOGGING_H

#include <vector>
#include <memory>
#include "FixedBuffer.h"
#include "Thread.h"


//
class AsyncLogging {
public:
    AsyncLogging(std::string baseName,int flushInterval = 2);
    ~AsyncLogging(){
        if(running_){
            stop();
        }
    }

    void append(const char* logLine, int len);

    void start(){
        running_ = true;
        thread_.start();
        latch_.wait();
    }

    void stop(){
        running_ = false;
        cond_.notify();
        thread_.join();
    }



private:
    void threadFunc();

    //缓冲区
    typedef FixedBuffer<KLargeBuffer> Buffer;
    typedef std::vector<std::shared_ptr<Buffer> > BufferVector;
    typedef std::shared_ptr<Buffer> BufferPtr;

    const int flushInterval_{};
    bool running_;
    std::string baseName_;
    Thread thread_;
    MutexLock mutex_;
    Condition cond_;
    BufferPtr currentBuffer_;
    BufferPtr nextBuffer_;
    BufferVector buffers_;
    CountDownLatch latch_;          //计数器



};


#endif //HTTPSERVER_ASYNCLOGGING_H
