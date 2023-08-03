//
// Created by 梁磊磊 on 2023/7/18.
//

#ifndef HTTPSERVER_LOGSTREAM_H
#define HTTPSERVER_LOGSTREAM_H

#include "noncopyable.h"
#include "FixedBuffer.h"

//重载<<运算符进行格式化输出，同时有自己的缓冲区，用于把多个<<结果连成一块
class LogStream : noncopyable {
public:
    typedef FixedBuffer<KSmallBuffer> Buffer;

    LogStream& operator<<(bool v){
        buffer_.append(v ?"1":"0" ,1);
        return *this;               //链式法则
    }

    LogStream& operator<<(short);
    LogStream& operator<<(unsigned short);
    LogStream& operator<<(int);
    LogStream& operator<<(unsigned int);
    LogStream& operator<<(long);
    LogStream& operator<<(unsigned long);
    LogStream& operator<<(long long);
    LogStream& operator<<(unsigned long long);

    LogStream& operator<<(const void*);

    LogStream& operator<<(float v){
        *this<< static_cast<double>(v);
        return *this;
    }
    LogStream& operator<<(double);
    LogStream& operator<<(long double);

    LogStream& operator<<(char v){
        buffer_.append(&v,1);
        return *this;
    }

    LogStream& operator<<(const char* str){
        if(str){
            buffer_.append(str,strlen(str));
        }
        else{
            buffer_.append("(null)",6);
        }
        return *this;
    }

    LogStream& operator<<(const unsigned char* str){
        return operator<<(reinterpret_cast<const char*>(str));
    }

    LogStream& operator<<(const std::string& str){
        buffer_.append(str.c_str(),str.length());
        return *this;
    }

    //缓冲区的一些操作
    void append(const char* data,int len){ buffer_.append(data,len);}
    const Buffer& getBuffer()const { return buffer_;}
    void resetBuffer() { buffer_.resetCur();}

private:
    template<class T>
    void formatInteger(T);

    Buffer buffer_;
    static const int kMaxNumericSize = 32;          //数字所占字符串的位数最大是32

};


#endif //HTTPSERVER_LOGSTREAM_H
