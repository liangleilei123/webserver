//
// Created by 梁磊磊 on 2023/7/19.
//

#include <cassert>
#include <linux/unistd.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/prctl.h>
#include "Thread.h"
#include "CurrentThread.h"

namespace CurrentThread{
    __thread int t_cachedTid = 0;
    __thread char t_tidString[32];
    __thread int t_tidStringLength = 6;
    __thread const char* t_threadName = "default";
}

pid_t get_tid(){ return static_cast<pid_t>(::syscall(SYS_gettid));}

void CurrentThread::cacheTid() {
    if(t_cachedTid == 0){
        t_cachedTid = get_tid();
        t_tidStringLength = snprintf(t_tidString,sizeof t_tidString,"%5d ",t_cachedTid);
    }
}

//在线程中保留name,tid这些数据

struct ThreadData{
    typedef Thread::ThreadFunc ThreadFunc;
    ThreadFunc func_;
    std::string name_;
    pid_t* tid_;
    CountDownLatch* latch_;

    ThreadData(const ThreadFunc& func,const std::string& name,pid_t* tid,CountDownLatch* latch):
    func_(func),name_(name),tid_(tid),latch_(latch){

    }

    //线程的工作函数
    void runInThread(){
        *tid_ = CurrentThread::tid();
        tid_ = nullptr;
        latch_->countDown();
        latch_ = nullptr;

        CurrentThread::t_threadName = name_.empty() ? "Thread" : name_.c_str();
        prctl(PR_SET_NAME,CurrentThread::t_threadName);                     //调用底层端口设置线程名

        func_();        //执行工作函数
        CurrentThread::t_threadName = "finished";           //设置线程名
    }
};

void* startThread(void* obj){
    ThreadData* data = static_cast<ThreadData*>(obj);
    data->runInThread();
    delete data;
    return nullptr;
}



Thread::Thread(const Thread::ThreadFunc &func, const std::string &thName):
started_(false),
joined_(false),
pthreadID_(0),
pid_(0),
func_(func),
thName_(thName),
latch_(1){
    setDefaultName();
}

Thread::~Thread() {
    if(started_ && !joined_)
        pthread_detach(pthreadID_);
}

void Thread::setDefaultName() {
    if(thName_.empty()){
        char buf[32]{};
        snprintf(buf,sizeof buf,"Thread");
        thName_ = buf;
    }
}

int Thread::join() {
    assert(!started_);
    assert(!joined_);
    joined_ = true;
    return pthread_join(pthreadID_,NULL);
}

// 线程开始
void Thread::start() {
    assert(!started_);
    started_ = true;
    ThreadData* data = new ThreadData(func_,thName_,&pid_,&latch_);
    if(pthread_create(&pthreadID_,NULL,&startThread,data) == 0){
        latch_.wait();
        assert(pid_>0);
    }
    else{
        started_ = false;
        delete data;
    }

}

