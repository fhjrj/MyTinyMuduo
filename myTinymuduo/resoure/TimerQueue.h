#ifndef _TIMER_QUEUE_
#define _TIMER_QUEUE_

#include <set>
#include <vector>
#include <utility>
#include <memory>
#include "Channel.h"
#include <mutex>
#include <atomic>
#include "Callbacks.h"
#include "noncopyable.h"

class Eventloop;
class Timer;
class TimerId;

class TimerQueue :public noncopyable
{
    public:
    TimerQueue(Eventloop*);
    ~TimerQueue();
   
   TimerId addTimer(Timercallback cb,
                   Timestamp when,
                   double interval);
      void cancel(TimerId timerId);

    private:
    using Entry=std::pair<Timestamp,Timer*>;//之所以设计如此是为了处理两个同时到期的任务，即使Timestamp相同 但是Timer*不同
    using TimerList=std::set<Entry>;
    using ActiveTimer=std::pair<Timer*,int64_t>;
    using ActiveTimerSet=std::set<ActiveTimer>;
    typedef typename  TimerList::iterator Timer_it;
    typedef typename  ActiveTimerSet::iterator Active_it;
    void addTimerInLoop(Timer* timer);
    void cancelInLoop(TimerId timerid);
    void handleRead();//  called when timerfd alarms
    std::vector<std::pair<Timestamp,Timer*>> getExpired(Timestamp now);//  // move out all expired timers
    void reset(const std::vector<Entry>& expired,Timestamp now);

   bool insert(Timer* timer);
   Eventloop* loop_;//mainloop
   const int timerfd_;
   channel timerfdChannel_;
   TimerList timers_;  
   ActiveTimerSet activeTimers_;//储存活跃的TIMER，发生了事件的Timer
   ActiveTimerSet caneclingTimers_;//储存没有存在在set集合中但是想要删除的元素(可能先前已经删除了或者从未加入)
   std::atomic<bool> callingExpiredTimers_;//待删队列 防止删除时任务还在回调 这个注意，所以每次进行回调时都会将其清空
};



#endif