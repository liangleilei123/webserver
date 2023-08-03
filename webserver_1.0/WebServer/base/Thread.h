//
// Created by 梁磊磊 on 2023/7/19.
//

#ifndef HTTPSERVER_THREAD_H
#define HTTPSERVER_THREAD_H

#include <string>
#include <functional>
#include "CountDownLatch.h"

class Thread:noncopyable {
public:
    typedef std::function<void()> ThreadFunc;
    explicit Thread(const ThreadFunc&,const std::string& thName = std::string());
    ~Thread();
    
    void start();
    int join();
    bool isStarted() const { return started_;}
    pid_t get_tid() const { return pid_;}
    const std::string& get_name() const { return thName_;}
    
    
private:
    void setDefaultName();
    
    bool started_;
    bool joined_;
    ThreadFunc func_;
    pthread_t pthreadID_;
    pid_t pid_;
    std::string thName_;            //当前线程的名字
    CountDownLatch latch_;
};


#endif //HTTPSERVER_THREAD_H
