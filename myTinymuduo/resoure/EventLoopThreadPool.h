#ifndef THREADPOLL
#define THREADPOLL

#include <functional>
#include <memory>
#include <vector>
#include <string>
#include <atomic>
#include "noncopyable.h"
#include "EventLoopThread.h"

class EventLoopThreadPoll :public noncopyable
{
    public:
    using ThreadInitCallback=std::function<void(Eventloop*)>;
    EventLoopThreadPoll(Eventloop* baseloop,const std::string& name);
    ~EventLoopThreadPoll();

    void setThreadNum(int number){
        numThread_=number;
    }

    void start(const ThreadInitCallback& cb);
     
     Eventloop* getNextLoop();//轮训方式投递任务
     std::vector<Eventloop*> getALLloops();

     bool started() const{
        return started_;
     }

     const std::string& name() const{
        return  name_;
     }

    private:
    bool started_;
    int numThread_;
    int next_;
    const std::string name_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<Eventloop*> loops_;
    Eventloop* baseloop_;//mainloop
    //析构时 线程类被std::unique_ptr<>回收------>当然要先等待std::shared_ptr<thread>回收------>先要等待线程任务执行完毕
};
//baseloop要用户在外面自己进行创建传入，在里面才会创建SUBLOOP
#endif