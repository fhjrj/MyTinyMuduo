#ifndef _EVENTLOOPTHREAD_
#define _EVENTLOOPTHREAD_

#include "noncopyable.h"
#include "Thread.h"
#include "Eventloop.h"
#include <functional>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <string>
class EventLoopThread : public noncopyable
{
   public:
   using ThreadInitCallback=std::function<void(Eventloop*)>;
   EventLoopThread(const ThreadInitCallback& cb=ThreadInitCallback(),const std::string& name=std::string());
   ~EventLoopThread();
   Eventloop* startloop();//开启循环

   private:
   void threadFunc();
   Eventloop* loop_;
   bool exiting_;
   Thread thread_;
   std::mutex m_mutex;
   std::condition_variable m_cond;
   ThreadInitCallback callback_;
};
#endif