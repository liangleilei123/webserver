//
// Created by 梁磊磊 on 2023/7/28.
//

#ifndef WEBS_MACROS_H
#define WEBS_MACROS_H

#include <cassert>
#include <stdexcept>

#define OS_linux

#define DISALLOW_COPY(cname) \
    cname(const cname &) = delete; \
    cname &operator=(const cname &) = delete;


#define DISALLOW_MOVE(cname) \
    cname(cname &&) = delete;   \
    cname & operator=(cname &&) = delete;

#define DISALLOW_COPY_AND_MOVE(cname) \
    DISALLOW_COPY(cname);                   \
    DISALLOW_MOVE(cname);


#define ASSERT(expr,message) assert((expr)&&(message))

#define UNREACHABLE(message) throw std::logic_error(message)

#endif //WEBS_MACROS_H
