//
// Created by 梁磊磊 on 2023/7/18.
//

#ifndef HTTPSERVER_CONDITION_H
#define HTTPSERVER_CONDITION_H

#include <errno.h>
#include <time.h>
#include "MutexLock.h"

class Condition:noncopyable{
public:
    explicit Condition(MutexLock &mutex)
    :mutex_(mutex){
        pthread_cond_init(&cond_,nullptr);
    }
    ~Condition(){ pthread_cond_destroy(&cond_);}

    void wait(){ pthread_cond_wait(&cond_,mutex_.get());}
    void notify(){ pthread_cond_signal(&cond_);}
    void notifyAll(){ pthread_cond_broadcast(&cond_);}
    //等待n秒后，
    bool waitForSeconds(int seconds){
        struct timespec absTime;
        clock_gettime(CLOCK_REALTIME,&absTime);
        absTime.tv_sec += static_cast<time_t>(seconds);
        return ETIMEDOUT == pthread_cond_timedwait(&cond_,mutex_.get(),&absTime);
    }

private:
    MutexLock &mutex_;
    pthread_cond_t cond_;
};

#endif //HTTPSERVER_CONDITION_H
