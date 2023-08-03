//
// Created by 梁磊磊 on 2023/8/2.
//

#include <sys/time.h>
#include <iostream>
#include "Timer.h"
#include "HttpConnect.h"

TimerNode::TimerNode(HttpConnect *httpConnect, int timeout)
        : httpConnect_(httpConnect),
          deleted_(false) {

    struct timeval now;
    gettimeofday(&now, NULL);
    //以毫秒计
    expiredTimer_ = (((now.tv_sec % 1000) * 1000) + (now.tv_usec / 1000)) + timeout;
}

TimerNode::~TimerNode() {
    if (httpConnect_) {
        httpConnect_->handleClose();
    }
}

TimerNode::TimerNode(TimerNode &tn)
        : httpConnect_(tn.httpConnect_),
          expiredTimer_(0),
          deleted_(tn.deleted_) {

}

void TimerNode::updateTimer(int timeout) {
    struct timeval now;
    gettimeofday(&now, NULL);
    expiredTimer_ =
            (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
}

bool TimerNode::isValid() {
    struct timeval now;
    gettimeofday(&now, NULL);
    size_t temp = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000));
    if (temp < expiredTimer_)
        return true;
    else {
        this->setDeleted();
        return false;
    }
}

void TimerNode::clearReq() {
    httpConnect_ = nullptr;
    this->setDeleted();
}

void TimerNode::setDeleted() {
    deleted_ = true;
}

bool TimerNode::isDeleted() const {
    return deleted_;
}

size_t TimerNode::getExpiredTimer() const {
    return expiredTimer_;
}

TimerManager::TimerManager()
{

}

TimerManager::~TimerManager() {

}

void TimerManager::addTimer(HttpConnect *httpConnect, int timeout) {
    std::unique_lock<std::mutex> lock(timerQueueMutex_);
//    TimerNode *newNode = new TimerNode(httpConnect, timeout);
//    timerNodeQueue_.push(newNode);
//    httpConnect->linkTimer(newNode);
    std::shared_ptr<TimerNode> newNode(new TimerNode(httpConnect,timeout));
    timerNodeQueue_.push(newNode);
    httpConnect->linkTimer(newNode);
}


void TimerManager::handleExpiredEvent() {
    std::unique_lock<std::mutex> lock(timerQueueMutex_);
    while (!timerNodeQueue_.empty()) {
        SPTimerNode ptimer_now = timerNodeQueue_.top();
        if (ptimer_now->isDeleted())
            timerNodeQueue_.pop();
        else if (ptimer_now->isValid() == false)
            timerNodeQueue_.pop();
        else
            break;
    }
}


