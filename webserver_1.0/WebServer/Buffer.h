//
// Created by 梁磊磊 on 2023/7/9.
//

#ifndef WEBS_BUFFER_H
#define WEBS_BUFFER_H


#include <string>
#include "Macros.h"

class Buffer {
public:
    Buffer();
    ~Buffer();

    void append(const char* str,ssize_t);
    ssize_t size();
    const char* c_str();
    void clear();
    void getLine();
    void setBuf(const char* );


private:
    DISALLOW_COPY_AND_MOVE(Buffer);
    std::string buf_;
};


#endif //WEBS_BUFFER_H
