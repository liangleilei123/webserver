//
// Created by 梁磊磊 on 2023/7/18.
//

#ifndef HTTPSERVER_LOGFILE_H
#define HTTPSERVER_LOGFILE_H

#include <memory>
#include "FileUtil.h"
#include "MutexLock.h"

class LogFile :noncopyable{
public:
    //每被append flushEveryN次，flush一下，会往文件写，只不过，文件也是带缓冲区的
    LogFile(const std::string &baseName,int flushEveryN = 1024);
    ~LogFile();

    void append(const char *logLine,int len);
    void flush();
    bool rollFile();

private:
    void append_unlocked(const char *logLine,int len);

    const std::string baseName_;
    const int flushEveryN_;

    int count_;             //循环次数
    std::unique_ptr<MutexLock> mutex_{nullptr};
    std::unique_ptr<AppendFile> file_{nullptr};

};


#endif //HTTPSERVER_LOGFILE_H
