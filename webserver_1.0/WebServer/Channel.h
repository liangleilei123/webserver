//
// Created by 梁磊磊 on 2023/7/3.
//

#ifndef WEBS_CHANNEL_H
#define WEBS_CHANNEL_H

#include <memory>
#include <functional>
#include "HttpData.h"
#include "Macros.h"



class EventLoop;

class HttpData;

class Epoll;

//用于在某些事件发生时通知应用程序进行相应的处理
//只维护一个线程
class Channel {
public:

    typedef std::function<void()> callBack;


    Channel(EventLoop*,int fd);           // 新建一个Channel时，必须说明该Channel与哪个epoll和fd绑定
    ~Channel();

    int getFd() const;
    uint32_t getEvents() const;
    uint32_t getReadyEvents() const;
    bool getInEpoll() const;
    void useET();                           //使用边沿触发方式

    void setInEpoll();
    void setEvents(uint32_t event);
    void setReadyEvents(uint32_t);
    void setReadCallBack(callBack);
    void setUseThreadPool(bool use = true);

    void handleEvent();
    void enableReading();                   //处理读事件

private:
    DISALLOW_COPY_AND_MOVE(Channel);

    EventLoop *loop_;

    __uint32_t fd_;             //线程的文件描述符
    __uint32_t events_;        //events表示希望监听这个文件描述符的哪些事件，因为不同事件的处理方式不一样。
    __uint32_t readyEvent;         //revents表示在epoll返回该Channel时文件描述符正在发生的事件。
    bool inEpoll_;
    bool useThreadPool_;

    callBack  readCallBack;
    callBack  writeCallBack;

};

typedef std::shared_ptr<Channel> SP_Channel;


#endif //WEBS_CHANNEL_H
