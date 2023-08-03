//
// Created by 梁磊磊 on 2023/8/2.
//

#ifndef HTTPTEST_TIMER_H
#define HTTPTEST_TIMER_H


#include <ctime>
#include <queue>
#include <mutex>
#include <memory>

class HttpConnect;

class TimerNode {
public:
    TimerNode(HttpConnect* httpConnect,int timeout);
    ~TimerNode();
    TimerNode(TimerNode &tn);

    void updateTimer(int timeout);
    bool isValid();
    void clearReq();
    void setDeleted();
    bool isDeleted()const;
    size_t getExpiredTimer()const;

private:
    bool deleted_;
    size_t expiredTimer_;
    HttpConnect* httpConnect_;
};

//struct TimerCmp{
//    bool operator()(const TimerNode* a,const TimerNode* b) const{
//        std::cout<< "Timer.h line:36 TimerCmp" <<std::endl;
//        return a->getExpiredTimer() > b->getExpiredTimer();
//    }
//};

struct TimerCmp {
    bool operator()(std::shared_ptr<TimerNode> &a,
                    std::shared_ptr<TimerNode> &b) const {
        return a->getExpiredTimer() > b->getExpiredTimer();
    }
};

class TimerManager{
public:
    TimerManager();
    ~TimerManager();

    void addTimer(HttpConnect* httpConnect,int timeout);
    void handleExpiredEvent();
private:

//    std::priority_queue<TimerNode* ,std::deque<TimerNode*>,TimerCmp> timerNodeQueue_;
    typedef std::shared_ptr<TimerNode> SPTimerNode;
    std::priority_queue<SPTimerNode,std::deque<SPTimerNode>,TimerCmp> timerNodeQueue_;
    std::mutex timerQueueMutex_;
};


#endif //HTTPTEST_TIMER_H
