#include "TimerQueue.h"
#include "logger.h"
#include "Timer.h"
#include "Timerid.h"
#include "Eventloop.h"
#include <sys/timerfd.h>
#include <iostream>
#include <algorithm>
#include <assert.h>
#include "Timestamp.h"
#include <unistd.h>
#include <strings.h>

 
 int createTimerfd()
 {
    int timerfd=::timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK|TFD_CLOEXEC);
    if(timerfd<0)
    {
        LOG_ERROR("TimerQueue ::timer_create :  Failed in timer_create \n");
    }
    return timerfd;
 }
 
struct timespec howMuchTimeFromNow(Timestamp when)
{
    int64_t time_=when.gettime()-Timestamp::now().gettime();//这个事件一定延迟的

    if(time_<100)
    {
        time_=100;
    }

    struct timespec ts;
    ts.tv_sec=static_cast<time_t>(time_/Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec=static_cast<long>((time_%Timestamp::kMicroSecondsPerSecond)*1000);
    return ts;//返回事件间隔
}

void readTimerfd(int timerfd,Timestamp now)
{
    uint64_t howmany;
    ssize_t n=::read(timerfd,&howmany,sizeof(howmany));//有事件，通知一个8字节的长度
    LOG_INFO("TimerQueue::handleRead() at %s \n",now.Tostring().c_str());
    if(n!=sizeof(howmany))
    {
        LOG_ERROR("TimerQueue ::handleread() reads instead of 8 ");
    }
}////----->EPOLL监听到了始终超时,进行调用handleread()------>从而进行回到readTimefd()


void resetTimerfd(int timerfd, Timestamp expiration)
{
    struct itimerspec newValue;
    struct itimerspec oldValue;
    memset(&newValue,0,sizeof(newValue));
    memset(&oldValue,0,sizeof(oldValue));
    newValue.it_value=howMuchTimeFromNow(expiration);//启动一次闹钟
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    /*定时器启动，调用函数it_value时间后时钟发出一次信号*/
    if(ret)
    {
        LOG_INFO("timerfd_settime()");
    }
}


TimerQueue::TimerQueue(Eventloop* loop):loop_(loop),
timerfd_(createTimerfd()),
timerfdChannel_(loop,timerfd_),
timers_(),
callingExpiredTimers_(false)
{
     timerfdChannel_.setreadcallback(
      std::bind(&TimerQueue::handleRead, this));
       timerfdChannel_.enableReading();//设置回调
}

TimerQueue::~TimerQueue()
{
    timerfdChannel_.disableAll();
    timerfdChannel_.remove();
    ::close(timerfd_);
    for(const Entry& timer :timers_)
    {
        delete timer.second;
    }
    //消除set中所有的定时器结点
}

void TimerQueue::cancel(TimerId timerId)
{
    loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop,this,timerId));
}

TimerId TimerQueue::addTimer(Timercallback cb,Timestamp now,double interval)
{
    Timer* timer=new Timer(std::move(cb),now,interval);//记录回调和编号和延长时间
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop,this,timer));//若是单个线程则不用管，调用后立刻调用addTimerLoop()，将时间结点
    //加入并且设置定时器，若是多线程一定确保开启loop()
    return TimerId(timer,timer->sequence());//进行TimerId进行封装timer
}

//Timerid抽象出来记载timer和其序列号

void TimerQueue::addTimerInLoop(Timer* timer)//runinLoop中执行插入
{
   bool ear=insert(timer);
   if(ear)
   {
    resetTimerfd(timerfd_,timer->expiration());//加入的任务的执行时间是最小的一个，或者时第一个，则以当前时间和执行此任务的时间差为定时器的超时时间处理任务,其他的则继续往后添加
   }
}


//set插入时会自动排序，若要插入自定义类，则需要重载运算符
bool TimerQueue::insert(Timer* timer){
    bool ear=false;
    Timestamp when=timer->expiration();
     //每一次插入时，两个SET的长度相同
    assert(timers_.size() == activeTimers_.size());
     Timer_it  it=timers_.begin();
    if(it==timers_.end()||when<(it->first))
    {
        ear=true;//第一个任务或者是第一个即将要执行的
    }
    {
        std::pair<Timer_it,bool> result=timers_.insert(Entry(when,timer));
        assert(result.second);
        (void)result;
    }
    {
        std::pair<Active_it,bool> result=activeTimers_.insert(ActiveTimer(timer,timer->sequence()));//加入活跃连接
          assert(result.second);
          (void)result;
    }
      assert(timers_.size() == activeTimers_.size());
    return ear;
    
}

std::vector<std::pair<Timestamp,Timer*>> TimerQueue::getExpired(Timestamp now)//获得所有过期的任务
{
      assert(timers_.size() == activeTimers_.size());
      std::vector<Entry> expired;
      std::shared_ptr<Timer> newtimer(new Timer());
      
      Entry sendtry(now,newtimer.get());
      
     Timer_it end = timers_.lower_bound(sendtry);//二分查找 找第一个>=的数，并返回其下标，pair<>比较的是key  

      assert(end == timers_.end() || now < end->first); 
      for(auto p:timers_)
      {
        if(p.first.gettime()>end->first.gettime())  break;
        expired.push_back(p);
      }
      for(auto mp :expired)
      {
        timers_.erase(mp);
        ActiveTimer timer__(mp.second,mp.second->sequence());
        activeTimers_.erase(timer__);
      }
      assert(timers_.size()==activeTimers_.size());
      return expired;
 
}


void TimerQueue::reset(const std::vector<Entry>& expired,Timestamp now){
       Timestamp nextEx;
       for(const Entry& it:expired)
       {
        ActiveTimer timer(it.second,it.second->sequence());
        if(it.second->repeat()&&caneclingTimers_.find(timer)==caneclingTimers_.end())//因为确保要重置，所以进行检查是否进行在取消队列中
        {//如果不在取消队列中才进行重置
            it.second->restart(now);//延长存活/执行时间
            insert(it.second);
        }
        else{
            delete it.second;//若不需要重置，则进行删除
        }
       }
       if(!timers_.empty())
       {
        nextEx=timers_.begin()->second->expiration();//记录第一个的执行时间
       }

       if(nextEx.gettime()>0)
       {
        resetTimerfd(timerfd_,nextEx);//重新设定定时器时间，始终以最小的进行定时
       }
}//以第一个时间为执行时间点

//等到定时器发出信号后，timerfd检测到读事件，然后回调handleRead()
void TimerQueue::handleRead()
{
   Timestamp now(Timestamp::now());
   readTimerfd(timerfd_,now);//
   std::vector<Entry> expired=getExpired(now);//找到所有过期的任务
   callingExpiredTimers_=true;//进行回调
   caneclingTimers_.clear();//
   for(const Entry& it:expired)
   {
    it.second->run();
   }
   callingExpiredTimers_=false;
   reset(expired,now);//执行完毕后，将过期的任务进行重置
}

void TimerQueue::cancelInLoop(TimerId timerid)//人为主动调用删除结点
{
     assert(timers_.size() == activeTimers_.size());
     ActiveTimer timer(timerid.timer_,timerid.sequence_);//提取出要删除的结点
     Active_it it=activeTimers_.find(timer);
     if(it!=activeTimers_.end())//说明存在
     {
          size_t n=timers_.erase(Entry(it->first->expiration(),it->first));
          assert(n==1);
          (void)n;
          delete it->first;//从timers中删除,然后删除Timer*定时器
          activeTimers_.erase(it);//两个表中都进行删除
     }
     else if(callingExpiredTimers_)//此说明正在执行回调（结点可能在进行回调。可能早已被删除，可能未被加入，都一律加入），则将其重新加入待删队列中
     {
     caneclingTimers_.insert(timer);
     }
       assert(timers_.size() == activeTimers_.size());
    
}
