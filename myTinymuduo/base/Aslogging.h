#ifndef ASYNC_LOG_H
#define ASYNC_LOG_H
#include "LogStream.h"
#include "Thread.h"
#include "noncopyable.h"
#include <memory>
#include <atomic>
#include <vector>
#include <string>
#include <mutex>
#include <condition_variable>
class AsyncLogging :public noncopyable
{  
    public:
   AsyncLogging(const std::string& basemae=std::string(),off_t rollSize=0,int flushInterval=3);

   ~AsyncLogging()
   {
    if(running_)
    {
        stop();
    }
   }
void append(const char* logline, int len);

  void start()
{
    running_=true;
    thread_.start();
}

void stop()
{
    running_=false;
    cond_.notify_all();
    thread_.join();
}
 void threadFunc();

    private:
    using Buffer_=FixedBuffer<KLargeBuffer>;
    using BufferVector=std::vector<std::unique_ptr<Buffer_>>;
    using BufferPtr=std::unique_ptr<Buffer_>;//std::unique_ptr<Buffer>

    const int flushInterval_;
    std::atomic<bool> running_;
    const std::string basename_;
    const off_t rollSize_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    BufferPtr currentBuffer_;
    BufferPtr nextBuffer_;
    BufferVector buffers_;
};
#endif