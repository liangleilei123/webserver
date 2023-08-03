//
// Created by 梁磊磊 on 2023/7/19.
//

#ifndef HTTPSERVER_CURRENTTHREAD_H
#define HTTPSERVER_CURRENTTHREAD_H

//记录线程信息，方便输出日志
namespace CurrentThread{
    extern __thread int t_cachedTid;             //缓存线程id
    extern __thread char t_tidString[32];       //线程id字符串
    extern __thread int t_tidStringLength;      //线程id字符串长度
    extern __thread const char* t_threadName;   //线程名称

    void cacheTid();
    inline int tid(){
        if(__builtin_expect(t_cachedTid==0,0)){          //__builtin_expect只在GCC编译器中有效，用于编译器优化
            cacheTid();
        }
        return t_cachedTid;
    }

    inline const char* tidString(){
        return t_tidString;
    }
    inline int tidStringLength(){
        return t_tidStringLength;
    }
    inline const char* threadName(){
        return t_threadName;
    }
}



#endif //HTTPSERVER_CURRENTTHREAD_H
