#ifndef _TIME_ID_
#define _TIME_ID_

#include <sys/types.h>
class Timer;
class TimerId//管理timer的类
{
  public:
  TimerId():timer_(nullptr),sequence_(0)
  {
  }
   
  TimerId(Timer* timer,int64_t seq):timer_(timer),
  sequence_(seq)
  {
  }
  friend class TimerQueue;//友元类 TimerQueue类可以访问 TimerID所有成员函数--->访问timer的所有成员
  private:
  Timer* timer_;
  int64_t sequence_;

};



#endif