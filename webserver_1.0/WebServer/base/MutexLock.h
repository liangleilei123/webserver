//
// Created by 梁磊磊 on 2023/7/18.
//

#ifndef HTTPSERVER_MUTEXLOCK_H
#define HTTPSERVER_MUTEXLOCK_H


#include <pthread.h>
#include "noncopyable.h"

//封装一个禁止赋值和拷贝的互斥锁对象，防止多个线程同时持有同一个互斥锁，从而引起竞争和死锁
class MutexLock : noncopyable {
public:
    MutexLock(){pthread_mutex_init(&mutex, nullptr);}
    ~MutexLock(){
        /*在销毁互斥锁之前先加锁，是为了确保在互斥锁被销毁之前，所有正在使用此锁的线程都已经退出临界区。
         如果不加锁直接销毁互斥锁，可能会发生以下情况：
            1、有线程正在临界区中使用锁，而此时锁已经被销毁，会导致临界区的数据出现问题。
            2、有线程正在等待此锁，而此时锁已经被销毁，会导致线程无法正常执行。
        */
        pthread_mutex_lock(&mutex);
        pthread_mutex_destroy(&mutex);
    }

    void lock(){
        pthread_mutex_lock(&mutex);
    }

    void unlock(){
        pthread_mutex_unlock(&mutex);
    }
    pthread_mutex_t* get(){
        return &mutex;
    }

private:
    pthread_mutex_t mutex;

    //友元类不受访问权限的影响
private:
    friend class Condition;
};

//互斥锁的RAII封装实现
class MutexLockGuard : noncopyable{
public:
    //这里是显式构造函数
    //在这个类中，引用类型和显式构造函数相结合，把mutex作为mutex_的别名，可以在这个类中通过mutex直接操作mutex_避免了拷贝构造。
    explicit MutexLockGuard(MutexLock& mutex_):mutex(mutex_){
        mutex.lock();
    }
    ~MutexLockGuard(){
        mutex.unlock();
    }
private:
    MutexLock& mutex;       //在这里定义MutexLock的引用类型
};



#endif //HTTPSERVER_MUTEXLOCK_H
