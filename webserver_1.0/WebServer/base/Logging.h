//
// Created by 梁磊磊 on 2023/7/20.
//

#ifndef HTTPSERVER_LOGGING_H
#define HTTPSERVER_LOGGING_H

#include "LogStream.h"

//内含一个LogStream对象，为了每次打log的时候在log之前和之后加上固定的格式化信息
class Logger {
public:
    Logger(const char* ,int);
    ~Logger();

    LogStream &stream(){ return impl_.stream__;}

    static void setLogFileName(std::string fileName){ logFileName_ = fileName;}
    static std::string getLogFileName(){ return logFileName_;}


private:
    class Impl{
    public:
        Impl(const char* ,int);
        void formatTime();

        LogStream stream__;
        int line__;
        std::string baseName__;
    };

    Impl impl_;
    static std::string logFileName_;
};
//这个宏定义使用了两个特殊的 C++ 预定义宏 __FILE__ 和 __LINE__。__FILE__ 表示当前源文件的文件名，__LINE__ 表示当前代码行号。
#define LOG Logger(__FILE__,__LINE__).stream()


#endif //HTTPSERVER_LOGGING_H
