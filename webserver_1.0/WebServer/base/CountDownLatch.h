//
// Created by 梁磊磊 on 2023/7/19.
//

#ifndef HTTPSERVER_COUNTDOWNLATCH_H
#define HTTPSERVER_COUNTDOWNLATCH_H


#include "MutexLock.h"
#include "Condition.h"

// 用于倒数并发操作的次数
// CountDownLatch的主要作用是确保Thread中传进去的func真的启动了以后外层的start才返回
class CountDownLatch:noncopyable {
public:
    explicit CountDownLatch(int count);
    void wait();
    void countDown();


private:
    mutable MutexLock mutex_;
    Condition cond_;
    int count_;
};


#endif //HTTPSERVER_COUNTDOWNLATCH_H
