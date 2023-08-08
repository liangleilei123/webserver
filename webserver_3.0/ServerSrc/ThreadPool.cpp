//
// Created by 梁磊磊 on 2023/8/2.
//

#include "ThreadPool.h"

ThreadPool::ThreadPool(int size)
:stop_(false)
{
    for(int i = 0; i < size; ++i){
        threads_.emplace_back(std::thread([this](){
            while(true){
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(tasksMutex_);
                    cond_.wait(lock, [this](){
                        return stop_ || !tasksQueue_.empty();
                    });
                    if(stop_ && tasksQueue_.empty()) return;
                    task = tasksQueue_.front();
                    tasksQueue_.pop();
                }
                task();
            }
        }));
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(tasksMutex_);
        stop_ = true;
    }
    cond_.notify_all();
    for(std::thread &th : threads_){
        if(th.joinable())
            th.join();
    }
}


