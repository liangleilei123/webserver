//
// Created by 梁磊磊 on 2023/7/18.
//

#ifndef HTTPSERVER_NONCOPYABLE_H
#define HTTPSERVER_NONCOPYABLE_H

// 禁止拷贝的父类
class noncopyable{

protected:
    //把构造和析构声明为保护类型，是为了限制这些函数的访问权限，确保只有该类及其子类可以访问它们。
    noncopyable(){}
    ~noncopyable(){}

private:
    //通过将复制构造函数和赋值运算符声明为private，并且不提供实现来实现禁止复制和赋值的效果。
    //如果继承noncopyable的子类没有重新定义复制构造函数和复制运算符，那么它们会被禁止复制和赋值。
    noncopyable(const noncopyable&);
    const noncopyable& operator=(const noncopyable&);
};

#endif //HTTPSERVER_NONCOPYABLE_H
