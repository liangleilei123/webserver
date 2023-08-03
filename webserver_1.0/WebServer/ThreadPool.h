//
// Created by 梁磊磊 on 2023/7/10.
//

#ifndef WEBS_THREADPOOL_H
#define WEBS_THREADPOOL_H

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>
#include <unordered_map>
#include "Macros.h"

class ThreadPool{

public:

    ThreadPool(int minThreadNums = 2,int maxThreadNums = 30);
    ~ThreadPool();

    template<class Func,class ...Args>
    auto addTask(Func&& func,Args&& ...args)->std::future<typename std::result_of<Func(Args...)>::type>{
        using return_type = typename std::result_of<Func(Args...)>::type;

        auto task = std::make_shared< std::packaged_task<return_type()> >(                    //使用智能指针
                std::bind(std::forward<Func>(func),std::forward<Args>(args)...)         //使用完美转发
                );
        std::future<return_type> res = task->get_future();                          //使用期约
        {       //队列锁作用域
            std::unique_lock<std::mutex> lock(mutex_);
            if(stop_){
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }
            tasksQ_.emplace([task](){
                (*task)();
            });
        }
        cond_.notify_one();
        return res;
    }

    int getBusyThreadNums();
    int getAliveThreadNums();


private:

    void manager();                         //管理函数
    void worker();
    void threadExit();                      //退出线程

private:
    DISALLOW_COPY_AND_MOVE(ThreadPool);

    std::mutex mutex_;                     //互斥量
    std::condition_variable cond_;               //条件变量

    ///临界区
    std::vector<std::thread> threads_;           //线程数组
    std::unordered_map<std::thread::id,std::thread*>  threads_map;
    std::queue<std::function<void()>> tasksQ_;    //任务队列
    bool stop_;
    int busyNum_;
    int aliveNum_;
    int minNum_;
    int maxNum_;
    int exitNum_;                               //要销毁的线程个数
    ///
    std::thread managerThread_;                 //管理者线程

};



#endif //WEBS_THREADPOOL_H
