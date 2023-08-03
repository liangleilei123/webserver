//
// Created by 梁磊磊 on 2023/7/18.
//
#include <cstdio>
#include "FileUtil.h"

// a表示以追加方式打开文件（即在文件末尾写入数据），e表示在打开文件时启用O_CLOEXEC选项，即在执行exec()系列函数时，自动关闭文件描述符。
AppendFile::AppendFile(std::string filename)
:fp_(fopen(filename.c_str(),"ae")){
    //设置文件缓冲区，提高文件读写的性能
    setbuffer(fp_,buffer_,sizeof buffer_);
}

AppendFile::~AppendFile() {
    fclose(fp_);        //fclose在关闭文件之前会自动释放缓冲区
}

size_t AppendFile::write(const char *logLine, size_t len) {
    return fwrite_unlocked(logLine,1,len,fp_);
}

void AppendFile::append(const char *logLine, const size_t len) {
    size_t write_size = write(logLine,len);
    size_t remain = len - write_size;
    while(remain > 0){
        size_t n = write(logLine,remain);
        if(n == 0){
            int err = ferror(fp_);
            if(err) fprintf(stderr,"AppendFile:append() failed!\n");
            break;
        }
        remain -= n;
    }
}

void AppendFile::flush() {
    fflush(fp_);
}
