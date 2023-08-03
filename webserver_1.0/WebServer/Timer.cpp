//
// Created by 梁磊磊 on 2023/7/3.
//

#include "Timer.h"
#include <sys/time.h>

TimerNode::TimerNode(TimerNode &tn)
        : expiredTime_(0),
          SPHttpData(tn.SPHttpData) {}

TimerNode::TimerNode(std::shared_ptr<HttpData> requestData, int timeout)
        : deleted_(false),
          SPHttpData(requestData) {
    struct timeval now;
    gettimeofday(&now,NULL);
    //超时事件以毫秒为单位计算
    expiredTime_ = (((now.tv_sec % 10000)*1000) + (now.tv_usec/1000)) + timeout;
}

TimerNode::~TimerNode() {
    if(SPHttpData){
        SPHttpData->handleClose();
    }
}

void TimerNode::update(int timeout){
    struct timeval now;
    gettimeofday(&now,NULL);
    expiredTime_ = (((now.tv_sec % 10000) * 1000) + now.tv_usec/1000)+timeout;
}

bool TimerNode::isValid() {
    struct timeval now;
    gettimeofday(&now,NULL);
    size_t temp = ((now.tv_sec % 10000)*1000) + now.tv_usec/1000;
    if(temp < expiredTime_){
        return true;
    }
    else{
        this->setDeleted();
        return false;
    }
}

void TimerNode::clearReg() {
    SPHttpData.reset();
    this->setDeleted();
}


void TimerManager::addTimer(std::shared_ptr<HttpData> SPHttpData, int timeout) {
    SPTimerNode new_node(new TimerNode(SPHttpData,timeout));        //智能指针的初始化
    timerNodeQueue.push(new_node);
    // SPHttpData->linkTimer(new_node);
}

//定时器管理的处理逻辑
/* 定时器优先队列中的定时器是按照超时时间排序的，因此可以通过判断队首元素是否过期来判断队列中是否存在已经过期的定时器。
 * 优先队列不支持随机访问，所以对于置为deleted的时间节点，会延迟到它超时或者它前面的节点都被删除时，它才会被删除
 * 一个点被置为deleted,它最迟会在TIMER_TIME_OUT时间后被删除。这么做有两个好处：
 * （1）不需要遍历优先队列，省时
 * （2）给超时时间一个容忍的时间，就是设定的超时时间是删除的下限(并不是一到超时时间就立即删除)，如果监听的请求在超时后的下一次请求中又一次出现了，
 *      就不用再重新申请RequestData节点了，这样可以继续重复利用前面的RequestData，减少了一次delete和一次new的时间。*/

void TimerManager::handleExpiredTime() {
    MutexLockGuard locker(lock);        //使用互斥锁，保证操作优先队列时是线程安全的
    while(!timerNodeQueue.empty()){
        SPTimerNode pTime_now = timerNodeQueue.top();
        if(pTime_now->isDeleted()){
            timerNodeQueue.pop();
        }
        else if(pTime_now->isValid() == false){
            //这里还可能返回回调函数
            timerNodeQueue.pop();
        }
        else{
            break;
        }
    }
}
