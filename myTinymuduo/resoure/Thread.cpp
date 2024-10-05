#include "Thread.h"
#include "CurrentThread.h"
#include <semaphore.h>
#include <assert.h>
std::atomic<int> Thread::numthread_(0);

Thread::Thread(ThreadFunc func, const std::string&name):
started_(false),
joined_(false),
tid_(0),
func_(std::move(func)),
name_(name)
{
    setdefaltname();
}

Thread::~Thread(){
    if(started_&&!joined_){
        thread_->detach();
    }
    }

void Thread::start(){
    started_=true;
    sem_t m_sem;
    sem_init(&m_sem,false,0);
    //std::atomic<int> p;
    thread_=std::shared_ptr<std::thread>(new std::thread([&](){
    tid_=CurrentThread::tid();
    //p.fetch_add(1,std::memory_order_release);
    sem_post(&m_sem);
    func_();
    }));
    sem_wait(&m_sem);//因为给每个线程都分配了ID 必须等ID值被创建了才能返回，这时重要的
    //assert(p.load(std::memory_order_acquire)!=0)
}
 
 void Thread::join(){
    started_=true;
    joined_=true;
    if(thread_->joinable())
     thread_->join();
 }

void Thread::setdefaltname() const{
    int num=++numthread_;
    if(name_.empty()){
        char buf[32]={0};//name_ const std::string
        snprintf(buf,sizeof(buf),"Thread%d",num);
        name_=buf;
    }
}