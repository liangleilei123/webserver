//
// Created by 梁磊磊 on 2023/7/18.
//

#ifndef HTTPSERVER_FIXEDBUFFER_H
#define HTTPSERVER_FIXEDBUFFER_H

#include <string>
#include <cstring>
#include "noncopyable.h"

const int KSmallBuffer = 4000;                  //4kb的内存空间
const int KLargeBuffer = 4000 * 1000;           //4MB的内存空间


template<int Size>
class FixedBuffer : noncopyable {
public:
    FixedBuffer() : cur_(data_) {}

    ~FixedBuffer() {}

    void append(const char *buf, size_t len) {
        if (remainSpace() > static_cast<int>(len)) {
            memcpy(cur_, buf, len);
            cur_ += len;
        }
    }

    const char *data() const { return data_; }

    int length() const { return static_cast<int>(cur_ - data_); }

    char *current() { return cur_; }

    int remainSpace() const{ return static_cast<int>(end() - cur_); }

    void add(size_t len) { cur_ += len; }

    void resetCur() { cur_ = data_; }

    void bzero() { memset(data_, 0, sizeof data_); }


private:
    const char *end() const { return data_ + sizeof data_; }

    char data_[Size];
    char *cur_;
};


#endif //HTTPSERVER_FIXEDBUFFER_H
