//
// Created by 梁磊磊 on 2023/7/18.
//

#include <cassert>
#include <iostream>
#include "AsyncLogging.h"
#include "LogFile.h"

AsyncLogging::AsyncLogging(std::string logFileName, int flushInterval):
running_(false),
flushInterval_(flushInterval),
baseName_(logFileName),
mutex_(),
cond_(mutex_),
latch_(1),
buffers_(),
currentBuffer_(new Buffer),
nextBuffer_(new Buffer),
thread_(std::bind(&AsyncLogging::threadFunc,this),"logging"){
    assert(logFileName.size() > 1);
    currentBuffer_->bzero();
    nextBuffer_->bzero();
    buffers_.reserve(16);
}

void AsyncLogging::append(const char *logLine, int len) {
    MutexLockGuard lock(mutex_);
    if(currentBuffer_->remainSpace() > len){
        currentBuffer_->append(logLine,len);
    }
    else{
        buffers_.push_back(currentBuffer_);
        currentBuffer_.reset();
        if(nextBuffer_){
            currentBuffer_ = std::move(nextBuffer_);
        }
        else{
            currentBuffer_.reset(new Buffer);
        }
        currentBuffer_->append(logLine,len);
        cond_.notify();
    }
}

//在这个线程工作函数中，主要负责在其他线程向缓冲区写入日志后，经由这个函数写入日志文件
void AsyncLogging::threadFunc() {
    assert(running_ == true);
    latch_.countDown();         //倒计时门闩可以保证异步日志线程已经启动并初始化完成
    LogFile outputFile(baseName_);              //初始化输入到文件的对象
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);                   //对应currentBuffer和nextBuffer
    newBuffer1->bzero();
    newBuffer2->bzero();
    BufferVector buffersToWrite;
    buffersToWrite.reserve(16);                         //效率更高

    while(running_){
        //std::cout<< newBuffer1->length()<<std::endl;
        assert(newBuffer1 && newBuffer1->length() == 0);
        assert(newBuffer2 && newBuffer2->length() == 0);
        assert(buffersToWrite.empty());                     //如果日志缓冲区是空的，说明日志系统有问题

        {
            MutexLockGuard lock(mutex_);
            if (buffers_.empty()) {
                cond_.waitForSeconds(flushInterval_);
            }
            buffers_.push_back(currentBuffer_);
            currentBuffer_.reset();

            currentBuffer_ = std::move(newBuffer1);
            buffersToWrite.swap(buffers_);      //将之前存储在 buffers_ 中的缓冲区移动到 buffersToWrite 中，以便在后续的日志写入过程中使用

            if (!nextBuffer_) {                           //nextBuffer_为空
                nextBuffer_ = std::move(newBuffer2);
            }
        }

        assert(!buffersToWrite.empty());

        if(buffersToWrite.size()>25){
            // char buf[256];
            // snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger
            // buffers\n",
            //          Timestamp::now().toFormattedString().c_str(),
            //          buffersToWrite.size()-2);
            // fputs(buf, stderr);
            // output.append(buf, static_cast<int>(strlen(buf)));
            buffersToWrite.erase(buffersToWrite.begin() + 2,buffersToWrite.end());
        }

        for(size_t i = 0;i < buffersToWrite.size();++i){
            outputFile.append(buffersToWrite[i]->data(),buffersToWrite[i]->length());
        }


        if(buffersToWrite.size() > 2){
            //丢弃非缓冲区
            buffersToWrite.resize(2);
        }

        if(!newBuffer1){
            assert(!buffersToWrite.empty());
            newBuffer1 = buffersToWrite.back();
            buffersToWrite.pop_back();
            newBuffer1->resetCur();
        }

        if(!newBuffer2){
            assert(!buffersToWrite.empty());
            newBuffer2 = buffersToWrite.back();
            buffersToWrite.pop_back();
            newBuffer2->resetCur();
        }

        buffersToWrite.clear();
        outputFile.flush();

    }
    outputFile.flush();

}



