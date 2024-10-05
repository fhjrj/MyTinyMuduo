#ifndef TIMER_T
#define TIMER_T

#include "Timestamp.h"
#include "Callbacks.h"
#include "noncopyable.h"
#include <atomic>
#include <memory>



class Timer: public noncopyable
{
   public:
    static std::atomic<int64_t>  s_numCreated;//计数 总的数量 
    Timer():
    expiration_(Timestamp::now()),
    interval_(0),
    repeat_(false),
    sequence_(0)
    {  
    }

    Timer(Timercallback cb,Timestamp when ,double interval):
    expiration_(when),
    callback_(std::move(cb)),
    interval_(interval),
    repeat_((when.gettime()>0.0?true:false)),//默认下都设置重置
    sequence_(0)
    {
    s_numCreated++;
    sequence_=static_cast<const int64_t>(s_numCreated.load());
    }

    void run() const{
        callback_();
    }
    void restart(Timestamp now){
        if(repeat_){
            expiration_=now.addTime(now,interval_);//扩展存活时间 并返回存活结束的时间
        }else{
            expiration_.invalude();
        }
    }
    int64_t sequence() const {return sequence_;}
    bool repeat() const {return repeat_;}
    Timestamp expiration() const {
        return expiration_;
    }
    static int64_t s_numCreated_() {return s_numCreated.load();}


    private:
    const Timercallback callback_;
    Timestamp expiration_;//加入时间/执行的绝对时间
     double interval_;//间隔时间 间隔回调
    const bool repeat_;//时间是否大于0
     int64_t sequence_;//时间类序号
};

 std::atomic<int64_t> Timer::s_numCreated(0);//计数 总的数量 
#endif


