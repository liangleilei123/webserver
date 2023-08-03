//
// Created by 梁磊磊 on 2023/7/10.
//

#include <unistd.h>
#include <iostream>
#include "ThreadPool.h"

//ThreadPool::ThreadPool(int minThreadNums,int maxThreadNums)
//:stop_(false),
//minNum_(minThreadNums),
//maxNum_(maxThreadNums){
//    for(int i = 0;i < minThreadNums;++i){
//        //lambda
//        threads_.emplace_back(std::thread([this]() {
//            while(true){
//                std::function<void()> task;
//                {
//                    std::unique_lock<std::mutex> taskLock(mutex_);
//                    cond_.wait(taskLock,[this]() ->bool {          //当ThreadPool关闭或任务队列不为空时，唤醒该线程
//                        return stop_ || !tasksQ_.empty();
//                    });
//                    if(stop_ && tasksQ_.empty()) return ;
//                    task = tasksQ_.front();
//                    tasksQ_.pop();
//                }       //使用代码块的吗目的是为了限制互斥量的作用域，从而减小互斥量对程序性能的影响，提高程序的并发性能。
//                task();
//            }
//        }));
//    }
//}

ThreadPool::ThreadPool(int minThreadNums,int maxThreadNums)
:stop_(false),
 minNum_(minThreadNums),
 maxNum_(maxThreadNums),
 aliveNum_(0){
    //先创建一个管理者线程
    managerThread_ = std::thread(std::bind(&ThreadPool::manager,this));
    //再创建最小数量的工作线程,把工作线程加入map中，方便销毁
    for(int i = 0;i < minNum_;++i){
        std::thread* thr = new std::thread(std::bind(&ThreadPool::worker,this));
        {
            std::unique_lock<std::mutex> threadLock(mutex_);
            threads_map[thr->get_id()] = thr;
            ++this->aliveNum_;
        }

    }

}

ThreadPool::~ThreadPool() {
//    {
//        std::unique_lock<std::mutex> lock(mutex_);
//        stop_ = true;
//    }
//    cond_.notify_all();
//    for(std::thread &th : threads_){
//        if(th.joinable()){          //回收线程
//            th.join();
//        }
//    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        stop_ = true;
    }
    managerThread_.join();
    for(int i = 0;i < aliveNum_;++i){
        cond_.notify_one();
    }


}

int ThreadPool::getBusyThreadNums() {
    int busyNum = 0;
    std::unique_lock<std::mutex> lock(mutex_);
    busyNum = busyNum_;
    lock.unlock();
    return busyNum;
}

int ThreadPool::getAliveThreadNums() {
    int aliveNum = 0;
    std::unique_lock<std::mutex> lock(mutex_);
    aliveNum = aliveNum_;
    lock.unlock();
    return aliveNum;
}

void ThreadPool::worker() {
     while(true){
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> taskLock(mutex_);
            cond_.wait(taskLock,[this]() ->bool {          //当ThreadPool关闭或任务队列不为空时，唤醒该线程
                return stop_ || !tasksQ_.empty();
            });
            if(exitNum_ > 0){
                --exitNum_;
                if(aliveNum_ > minNum_){
                    --aliveNum_;
                    taskLock.unlock();
                    threadExit();
                }
            }
//            if(stop_){
//                taskLock.unlock();
//                threadExit();
//            }
            if(stop_ && tasksQ_.empty()) return ;
            ++busyNum_;
            task = tasksQ_.front();
            tasksQ_.pop();
        }       //使用代码块的吗目的是为了限制互斥量的作用域，从而减小互斥量对程序性能的影响，提高程序的并发性能。
        task();

         {
             std::unique_lock<std::mutex> lock(mutex_);
             --this->busyNum_;
         }
    }
}

void ThreadPool::manager() {
    while(!stop_){
        //先获取线程相关的数据
        sleep(2);           //每两秒钟检查一次

        std::unique_lock<std::mutex> lock(mutex_);
        int taskNum = tasksQ_.size();
        int liveThreadsNum = this->aliveNum_;
        int busyThreadsNum = this->busyNum_;
        lock.unlock();

        const int NUMBER = 2;
        //如果线程不够，加线程
        if(taskNum > liveThreadsNum - busyThreadsNum && liveThreadsNum < maxNum_){
            for(int i = 0;i + aliveNum_ < maxNum_&& i < NUMBER;++i){
                std::thread* thr = new std::thread(std::bind(&ThreadPool::worker,this));
                lock.lock();
                threads_map[thr->get_id()] = thr;
                ++aliveNum_;
                lock.unlock();
            }
        }

        //如果线程过多，减线程
        if(busyThreadsNum * 2 < liveThreadsNum && liveThreadsNum > minNum_){
            {
                lock.lock();
                exitNum_ = NUMBER;
                lock.unlock();
            }
            //让工作的线程自杀
            for(int i = 0;i < NUMBER;++i){
                cond_.notify_one();                 //唤醒条件变量的线程，跳转到work函数看是否要销毁线程
            }
        }

    }
}

void ThreadPool::threadExit() {
    std::thread::id this_id = std::this_thread::get_id();
    if(threads_map.find(this_id) != threads_map.end()){
        std::cout<<"threadExit() function: thread "
                 << this_id <<" exiting... "<<std::endl;

        if(threads_map[this_id]->joinable()) {
            threads_map[this_id]->join();

            std::thread *thr = threads_map[this_id];
            {

                std::unique_lock<std::mutex> lock(mutex_);
                threads_map.erase(this_id);
            }
            delete thr;
        }
    }

}





