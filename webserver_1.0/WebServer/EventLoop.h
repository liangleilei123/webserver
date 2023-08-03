//
// Created by 梁磊磊 on 2023/7/1.
//

#ifndef WEBS_EVENTLOOP_H
#define WEBS_EVENTLOOP_H

#include <memory>           //智能指针
#include <vector>
#include <functional>
#include <cassert>
#include "Epoll.h"
#include "Channel.h"
#include "base/MutexLock.h"
#include "base/CurrentThread.h"
#include "ThreadPool.h"
#include "Macros.h"


class EventLoop{
public:
    EventLoop(int port);
    ~EventLoop();

    void loop();
    void updateChannel(Channel* channel);
    int getPort();
    void addThread(std::function<void()>);


private:
    DISALLOW_COPY_AND_MOVE(EventLoop);

    Epoll* epoll_;
    bool quit_;
    int port_;
    ThreadPool* threadPool_;

};

/*
//实现事件循环，处理网络事件和定时器事件。
//从事件分发器中获取事件并将其分发给对应的事件处理器进行处理。
class EventLoop {
    typedef std::function<void()> Functor;

    EventLoop();

    ~EventLoop();

    //开始循环 调用该函数的线程必须是该Eventloop所在的线程，也就是Loop函数不能跨线程使用
    void loop();

    //停止循环
    void stopLoop();

    //如果当前线程就是创建此EventLoop的线程 就调用callback(关闭连接 EpollDel) 否则就放入等待执行 函数区
    void runInLoop(Functor &&func);

    //把此函数放入等待执行函数区，如果当前是跨线程或者正在调用等待的函数，则唤醒
    void queueInLoop(Functor &&func);

    //把socket文件描述符fd和绑定的事件注册到poller内核时间表
    void pollerAdd(std::shared_ptr<Channel> channel, int timeout = 0);

    // 在poller内核事件表修改fd所绑定的事件
    void pollerMod(std::shared_ptr<Channel> channel, int timeout = 0);

    //从poller中删除fd及其绑定的事件
    void pollerDel(std::shared_ptr<Channel> channel);

    //只关闭连接(此时还可以把缓冲区数据写完再关闭)
    void shutDown(std::shared_ptr<Channel> channel);

    // 判断是否在loop的线程中
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

    void assertInLoopThread() { assert(isInLoopThread()); }

private:
    //创建eventfd 类似的管道的进程通信方式
    static int creatEventFd();

    //event fd的读回调函数(因为event_fd写了数据，所以触发可读事件，从event_fd读数据)
    void HandleRead();

    //event fd的更新事件回调函数（更新监听事件）
    void HandleUpdate();

    //异步唤醒SubLoop的epoll_wait(向event_fd中写入数据)
    void wakeUp();

    //执行正在等待的函数(SubLoop注册EpollAdd连接套接字以及绑定事件的函数)
    void performPendingFunctions();


private:
    //声明顺序 wakeUpFd_ > wakeUp_Channel_
    std::shared_ptr<Epoll> poller_;         //事件分发器，io多路复用
    //线程的一些数据成员
    pid_t threadId_;                      //线程ID
    mutable MutexLock mutex_;
    int wakeUpFd_;                        //用于异步唤醒 SubLoop 的 Loop 函数中的Poll(epoll_wait因为还没有注册fd会一直阻塞)
    std::shared_ptr<Channel> wakeUp_Channel_;        //用于异步唤醒Channel
    //处理事件
    std::vector<Functor> pending_functions_;    //正在等待的事件处理函数
    //处理事件的标志
    bool is_stop_;                              //是否停止事件循环
    bool is_looping_;                           //是否正在事件循环
    bool is_event_handling_;                     //是否事件正在处理
    bool is_calling_pending_functions_;         //是否正在调用等待处理的函数

};

 */

#endif //WEBS_EVENTLOOP_H
