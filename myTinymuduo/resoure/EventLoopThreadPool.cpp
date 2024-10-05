#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"
EventLoopThreadPoll::EventLoopThreadPoll(Eventloop* baseloop__,const std::string& name):
baseloop_(baseloop__),
name_(name),
started_(false),
numThread_(0),
next_(0)
{}

EventLoopThreadPoll::~EventLoopThreadPoll(){

}

void EventLoopThreadPoll::start(const ThreadInitCallback& cb){
    started_=true;
    for(int i=0;i<numThread_;i++){
        char buf[name_.size()+32];
        snprintf(buf, sizeof(buf), "%s%d", name_.c_str(), i);
         EventLoopThread* p=new EventLoopThread(cb,buf);//loop线程在其内部创建及局部的eventloop,然后进行looping,阻塞执行，结束后局部遍历自动被回收
         threads_.push_back(std::unique_ptr<EventLoopThread>(p));//管理线程类对象，
         loops_.push_back(p->startloop());//这里插入，函数返回 直接对应的subloop已经开始了

    }//没有开创线程 调用baseloop_
    if(numThread_==0&&cb){
        cb(baseloop_);
    }
}//创建subloop

 Eventloop* EventLoopThreadPoll::getNextLoop(){
    Eventloop* loop=baseloop_; 
    if(!loops_.empty()){
        loop=loops_[next_];
        ++next_;
        int m=static_cast<int>(loops_.size());
        if(next_>=m){
            next_=0;
        }
    }
    return loop;
 }
/*无创建线程 调用基类*/
 std::vector<Eventloop*> EventLoopThreadPoll::getALLloops(){
    if(loops_.empty()){
        return std::vector<Eventloop*>(1,baseloop_);
    }else{
        return loops_;
    }
 }

