//
// Created by 梁磊磊 on 2023/8/2.
//

#ifndef HTTPTEST_THREADPOOL_H
#define HTTPTEST_THREADPOOL_H


#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>

class ThreadPool {
public:
    ThreadPool(int size);
    ~ThreadPool();

    template<class F,class... Args>
    auto addTask(F&& f,Args&&... args)->std::future<typename std::result_of<F(Args...)>::type>;

private:
    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> tasksQueue_;
    std::mutex tasksMutex_;
    std::condition_variable cond_;
    bool stop_;
};

template<class F, class... Args>
auto ThreadPool::addTask(F &&f, Args &&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(tasksMutex_);
        if(stop_){
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        tasksQueue_.template emplace([task](){
            (*task)();
        });
    }
    cond_.notify_one();
    return res;
}


#endif //HTTPTEST_THREADPOOL_H
