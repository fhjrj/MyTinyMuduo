#ifndef _EPOLL_S_
#define _EPOLL_S_

#include <functional>
#include <atomic>
#include <vector>
#include <thread>
#include <mutex>

#include "Poller.h"
#include "CurrentThread.h"
#include "Callbacks.h"
#include <sys/types.h>
#include "Timestamp.h"
#include "Timerid.h"
#include "noncopyable.h"

class channel;
class TimerQueue;

/*epoll的抽象，实现I/O多路复用，循环事件监听*/
 class Eventloop{
 public:
 
Eventloop();
~Eventloop();//退出当前loop

void loop();//开启事件循环
void quit();//退出事件循环
using Functor=std::function<void()>;

 Timestamp pollReturnTime() const { return pollReturnTime_; }

 void runInLoop(Functor cb);//放在当前loop去执行

 void queueInLoop(Functor cb);//放入队列中，唤醒loop所在线程去执行

  void wakeup();//进行唤醒loop所在线程

  void updateChannel(channel* channel);
  void removeChannel(channel* channel);
  bool hasChannel(channel* channel);
  /*loop调用channel中的函数*/
  bool isInLoopThread() const { return threadID_ == CurrentThread::tid(); }
     /*判断是否在当前线程，若不在则只能在其本来线程中去唤醒*/
    
  size_t queueSize() const;
  
  TimerId runAt(Timestamp time,Timercallback cb);//Runs callback at 'time'.
  TimerId runAfter(double delay ,Timercallback cb);//Runs callback after @c delay seconds.
  TimerId runEvery(double interval,Timercallback cb);//Runs callback every @c interval seconds.
  void cancel(TimerId timerId);
    

 private:
 using ChannelList =std::vector<channel*>;
/*是否进行loop*/
 std::atomic<bool> looping_;
 std::atomic<bool> quit_;

 pid_t threadID_;//记录当前LOOP所在线程的ID
 Timestamp  pollReturnTime_;//返回发生事件的时间点
 std::unique_ptr<Poller> poller_;//继承 调用epollpoll
 int wakeupfd_;//当mainloop获得新用户channel时，通过轮训选择一个subloop,通过该成员唤醒subloop
 std::unique_ptr<channel> wakeupChannel_;
 ChannelList activechannels_;//当前loop管理的所有的channel
 channel* currentActivechannel_;

 std::atomic<bool> callingPendingFunctors_;//当前loop是否需要执行回调操作
 mutable std::mutex m_mutex;
 std::vector<Functor> pendingFunctions_;//储存loop的所有回调操作
 std::unique_ptr<TimerQueue> timerQueue_;
void handleRead();  // waked up
void doPendingFunctors();//进行回到函数用的

 };
/*一个线程有一个eventloop,一个poller,多个channel，事件循环监听channel,eventloop进行调用*/
/*直接操控poller*/

 #endif