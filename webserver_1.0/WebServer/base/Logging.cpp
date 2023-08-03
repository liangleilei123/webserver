//
// Created by 梁磊磊 on 2023/7/20.
//

#include <sys/time.h>
#include <time.h>
#include "Logging.h"
#include "AsyncLogging.h"

static pthread_once_t once_control_ = PTHREAD_ONCE_INIT;
static AsyncLogging *asyncLogger_;

std::string Logger::logFileName_ = "./WebServer.log";           //静态数据成员要在类外初始化
//std::string Logger::logFileName_ = "/home/WebServer.log";           //静态数据成员要在类外初始化


void once_init() {
    asyncLogger_ = new AsyncLogging(Logger::getLogFileName());
    asyncLogger_->start();
}

void output(const char *msg, int len) {
    pthread_once(&once_control_, once_init);     //保证once_init函数只被执行一次
    asyncLogger_->append(msg, len);
}

Logger::Impl::Impl(const char *filename, int line) :
        stream__(),
        line__(line),
        baseName__(filename) {
    formatTime();
}

void Logger::Impl::formatTime() {
    struct timeval tv;
    time_t time;
    char str_t[26]{};
    gettimeofday(&tv, NULL);
    time = tv.tv_sec;
    struct tm *p_time = localtime(&time);
    strftime(str_t, 26, "%Y-%m-%d %H:%M:%S\n", p_time);
    stream__ << str_t;
}

Logger::Logger(const char *fileName, int line) :
        impl_(fileName, line) {

}

Logger::~Logger() {
    impl_.stream__ << " -- " << impl_.baseName__ << ':' << impl_.line__ << '\n';
    const LogStream::Buffer& buf(stream().getBuffer());
    output(buf.data(),buf.length());
}