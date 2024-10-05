#include "EventLoopThread.h"
//一个LOOP对应一个线程

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,const std::string& name):
loop_(nullptr),
exiting_(false),
thread_(std::bind(&EventLoopThread::threadFunc,this),name),
m_mutex(),
callback_(cb),
m_cond()
{}

EventLoopThread::~EventLoopThread(){
    exiting_=true;
    if(loop_!=nullptr){
        loop_->quit();//线程退出，LOOP对应事件循环退出
        thread_.join();
    }
}

Eventloop* EventLoopThread::startloop(){
  thread_.start();//EventLoopThread::threadFunc并且开启一个线程
  Eventloop* loop=nullptr;
  {
   std::unique_lock<std::mutex> locker(m_mutex);
   while(loop_==nullptr){
    m_cond.wait(locker);
   }
    loop=loop_;
  }
  return loop;
}//

//其在单独的新线程中执行
void EventLoopThread::threadFunc(){
   Eventloop loop;//创建一个eventloop，和线程一一对应
   if(callback_){
    callback_(&loop);
   }//先进行线程的初始化回调

   {
 std::unique_lock<std::mutex> lockerp(m_mutex);
 loop_=&loop;
 m_cond.notify_one();
   }
  loop.loop();//---->一直循环，若结束，已经对象被销毁
  std::unique_lock<std::mutex> lockerr(m_mutex);
  loop_=nullptr;//结束对指针制空，安全
}

/*开始时绑定threadFunc()，startloop执行 创建一个线程调用threadFunc.执行startloop，若startloop()执行比线程调用的threadFunc()块，则会发现threadFunc还未创建LOOP对象，则会被阻塞*/