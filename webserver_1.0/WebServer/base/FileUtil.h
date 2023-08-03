//
// Created by 梁磊磊 on 2023/7/18.
//

#ifndef HTTPSERVER_FILEUTIL_H
#define HTTPSERVER_FILEUTIL_H

#include <string>
#include "noncopyable.h"


// 功能：直接向文件写入log
// 禁止赋值和拷贝
class AppendFile: noncopyable{

public:
    AppendFile(std::string filename);
    ~AppendFile();
    //append以行为单位向文件写
    void append(const char *logLine,const size_t len);
    void flush();
private:
    size_t write(const char *logLine,size_t len);
    FILE* fp_{nullptr};
    char buffer_[64*1024]{};      //64KB的缓冲区
};


#endif //HTTPSERVER_FILEUTIL_H
