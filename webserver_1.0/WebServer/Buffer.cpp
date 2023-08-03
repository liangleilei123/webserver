//
// Created by 梁磊磊 on 2023/7/9.
//

#include <iostream>
#include "Buffer.h"


Buffer::Buffer() {

}

Buffer::~Buffer() {

}

void Buffer::append(const char *str,ssize_t len) {
    for(int i = 0 ;i < len;++i){
        //这有没有别的方法，不用这个判断
        if(str[i] == '\0')
            break;
        buf_.push_back(str[i]);
    }
    //为什么不用buf.append(str);

}

ssize_t Buffer::size() {
    return buf_.size();
}

const char *Buffer::c_str() {
    return buf_.c_str();
}

void Buffer::clear() {
    buf_.clear();
}

void Buffer::getLine() {
    buf_.clear();
    std::getline(std::cin,buf_);
}

void Buffer::setBuf(const char* c_str) {
    buf_.clear();
    buf_.append(c_str);
}

