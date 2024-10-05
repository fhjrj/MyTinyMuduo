#include "Aslogging.h"
#include "Timestamp.h"
#include "LogFile.h"
#include <chrono>
#include <stdio.h>
#include <assert.h>

AsyncLogging::AsyncLogging(const std::string& basemae,off_t rollSize,int flushInterval):
  flushInterval_(flushInterval),
    running_(false),
    basename_(basemae),
    rollSize_(rollSize),
    thread_(std::bind(&AsyncLogging::threadFunc, this), "Logging"),
    mutex_(),
   cond_(),
    currentBuffer_(new Buffer_),
    nextBuffer_(new Buffer_),
    buffers_()
    {
        currentBuffer_->bzero();
        nextBuffer_->bzero();
        buffers_.reserve(16);
    }


 void AsyncLogging::append(const char* logline, int len)//用户调用
 {
    std::unique_lock<std::mutex> locker(mutex_);
    if(currentBuffer_->avail()>len)
    {
        currentBuffer_->append(logline,len);
    }else{
        buffers_.push_back(std::move(currentBuffer_));//满了 将其插入在BUFFER中
         
         if(nextBuffer_)//为空 说明threadFUNc()未来得及填充nextBuffer
         {
            currentBuffer_=std::move(nextBuffer_);//托付所有权，当前缓冲区空间不够，将新信息写入备用缓冲区
         }else{
            currentBuffer_.reset(new Buffer_);
         }

         currentBuffer_->append(logline,len);
         cond_.notify_one();//nextBuffer_=nullptr
    }//此函数结束currentbuffer一定是存在的，
 }


 void AsyncLogging::threadFunc()
 {
      assert(running_==true);
      LogFile output(basename_,rollSize_,false);//basename_ 一个完成的目录，/结尾,作为写入磁盘的接口
     BufferPtr newBUffer1(new Buffer_);
     BufferPtr newBUffer2(new Buffer_);
     newBUffer1->bzero();
     newBUffer2->bzero();
     BufferVector buffersToWrite;
     buffersToWrite.reserve(16);
     while(running_)
     {

         assert(newBUffer1 && newBUffer1->length() == 0);
         assert(newBUffer2 && newBUffer2->length() == 0);
         assert(buffersToWrite.empty());
        
        {
            std::unique_lock<std::mutex> locker(mutex_);
            cond_.wait_for(locker,std::chrono::seconds(flushInterval_),[this](){
                            return !this->buffers_.empty();
            });
            buffers_.push_back(std::move(currentBuffer_));
            currentBuffer_=std::move(newBUffer1);//移动后，currentbuffer_为空，使用预备缓冲区填充currentBuffer，newBUffer1_=nullptr
            buffersToWrite.swap(buffers_);
            if(!nextBuffer_)
            {
                nextBuffer_=std::move(newBUffer2);//填充nextBuffer
            }
        }

         for (const auto& buffer : buffersToWrite)
        {
            output.append(buffer->data(), buffer->length());
        }//代写数据全部写入

        if(buffersToWrite.size()>2)
        {
            buffersToWrite.reserve(2);//只留下两个预备区
        }

        if(!newBUffer1)//归还newBUffer1
        {
            newBUffer1=std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBUffer1.reset();//重新开辟空间
        }

        if(!newBUffer2)
        {
            newBUffer2=std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBUffer2.reset();////重新开辟空间
        }
        buffersToWrite.clear();//不能有残留！
        output.flush();
     }
      output.flush();
      }

      /*
      buffers_----->往文件中写入数据
      currentbuffer------>将日志写入其中，然后在插入buffers_
      nextBuffer-------->用于写入currentbuffer时进行填充currentbuffer
      newBuffer1------->补充currentbuffer
      newBuffer2------->填充nextbuffer
      currentbuffer约等于由三个buffer进行补充且互斥所以一定不为空
      */