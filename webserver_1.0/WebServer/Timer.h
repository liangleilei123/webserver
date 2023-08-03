//
// Created by 梁磊磊 on 2023/7/3.
//

#ifndef WEBS_TIMER_H
#define WEBS_TIMER_H

#include <cstddef>
#include <memory>
#include <queue>
#include "HttpData.h"
#include "base/MutexLock.h"

class HttpData;

//表示一个定时器节点，用于保存一个HttpData对象和其超时时间。
class TimerNode {
public:
    TimerNode(std::shared_ptr<HttpData> requestData, int timeout);

    TimerNode(TimerNode &tn);

    ~TimerNode();

    //更新定时器的超时时间，将当前时间加上超时时间得到新的超时时间。
    void update(int timeout);

    //检查定时器是否有效，即当前时间是否小于定时器的超时时间。
    bool isValid();

    //清空定时器节点中的 HttpData 对象，释放对象的资源。
    void clearReg();

    void setDeleted(){ deleted_ = true;}

    bool isDeleted()const{ return deleted_;}

    size_t getExpiredTime() const { return expiredTime_;}


private:
    bool deleted_;
    size_t expiredTime_;
    std::shared_ptr<HttpData> SPHttpData;
};


//TimerCmp类是一个比较函数对象，用于比较两个定时器节点的超时时间。struct TimerCmp{
struct TimerCmp {
    bool operator()(std::shared_ptr<TimerNode> &a,
            std::shared_ptr<TimerNode> &b) const {
        return a->getExpiredTime() > b->getExpiredTime();
    }
};

//TimerManager类是一个定时器管理器，用于管理所有的定时器节点。
class TimerManager {
public:
    TimerManager();
    ~TimerManager();
    void addTimer(std::shared_ptr<HttpData> SPHttpData,int timeout);        //添加定时器
    void handleExpiredTime();       //处理超时事件

private:
    typedef std::shared_ptr<TimerNode> SPTimerNode;
    std::priority_queue<SPTimerNode,std::deque<SPTimerNode>,TimerCmp > timerNodeQueue;
    MutexLock lock;             //该类中的 timerNodeQueue 需要保证线程安全，使用互斥锁来保护队列进行访问
};


#endif //WEBS_TIMER_H
