#include "Eventloop.h"
#include "logger.h"
#include <errno.h>
#include "TimerQueue.h"
#include "Channel.h"
#include <iostream>
#include <unistd.h>
#include <sys/eventfd.h>
__thread Eventloop* t_loopInThisthread=nullptr;//一个线程只能有一个eventloop对象

const int KPollTimeMs=10000;

int createEventfd(){
    int wakeup_fd=::eventfd(0,EFD_NONBLOCK|EFD_CLOEXEC);
     if(wakeup_fd<0)
     {
       LOG_FATAL("eventfd error %d \n",errno);
     }
     return wakeup_fd;
}
/*用来唤醒subreactor,处理新来的channel*/
//最开始调用构造函数的那一刻，该类就已经分配好整体内存和指定this指针.所以可以用this指针进行初始化
Eventloop::Eventloop():looping_(false),quit_(false),callingPendingFunctors_(false),
poller_(Poller::newDefaultPoller(this)),threadID_(CurrentThread::tid()),wakeupfd_(createEventfd()),
wakeupChannel_(new channel(this,wakeupfd_)),currentActivechannel_(nullptr),timerQueue_(new TimerQueue(this))
{
    LOG_DEBUG("EventLoop create %p in thread %d \n",this,threadID_);
    if(t_loopInThisthread){
        LOG_FATAL("Another EventLoop %p exits in this thread %d \n",t_loopInThisthread,threadID_);
        //此eventloop已经所属于其他线程，不能在此线程创建
    }else{
        t_loopInThisthread=this;
    }
 //设置wakeupfd的事件类型和回调操作 事件：唤醒subeventloop,执行事件，主loop唤醒
 wakeupChannel_->setreadcallback(std::bind(&Eventloop::handleRead,this));
 wakeupChannel_->enableReading();//channel::enablereading--->channel---->update---->eventloop::updatechannel---->poller::updatechannel
 //进行监听channel的读事件
}
/*刚开始时，先进行创建一个channel,此channel关联wakeupfd,通过此进行主从reactor交流*/
Eventloop::~Eventloop(){
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupfd_);//关闭主从连接
    t_loopInThisthread=nullptr;
}


void Eventloop::loop(){
    looping_=true;
    quit_=false;
    LOG_INFO("EventLoop %p start looping\n", this);
    while(!quit_){
        activechannels_.clear();
        /*监听两类fd,一个是channelfd,另一个是mainloop/subloop联系通信的fd*/
        /*将此loop当作subloop*/
        pollReturnTime_=poller_->poll(KPollTimeMs,&activechannels_);//调用epoll_wait,得到发生了事件的channel，但是有阻塞
        for(channel* channel__:activechannels_){
            channel__->handleEvent(pollReturnTime_);//进行回调,未执行tie
        }
        doPendingFunctors();//此函数作用就是   1.当主loop唤醒subloop，但此时还未加入channel,让subloop先回调先注册的事件
                           //别的线程调用此LOOP时，是不行的，要将调用函数加入到其中让此LOOP所属线程执行
    }
    LOG_INFO("Eventloop %p stop looping",this);
    looping_=false;
}

void Eventloop::quit(){//------->外部调用
    quit_=true;
    if(!isInLoopThread()){
        wakeup();
    }
}

void Eventloop::runInLoop(Functor cb)//----->外面调用，加入新的任务
{
    if(isInLoopThread())
    {
        cb();//是当前线程执行
    }else {//此loop不在自己的线程中，本线程唤醒此LOOP所属线程，将任务放入LOOP中的回调函数数组中，等待此LOOP所属线程去调用
     queueInLoop(cb);
    }
}
//B---bloop    
//A---aloop 这时A线程调用bloop的runInloop(cb),此任务应该是B线程执行，B一直执行loop（）,所以A将其加入到B线程所属的bloop中的任务队列中。
//但此时B线程可能阻塞在epoll_wait()中，因为要即使处理任务，所以进行wakeup(),向wakefp发送消息，epoll_wait()监听到了，立刻进行返回执行doPendingFunctors()
//对于quit()函数也是如此，A线程调用bloop()此时，B进行loop() 情况一样
//因为wakefd都加入到自己所属的进行监听了。
void Eventloop::queueInLoop(Functor cb)
{
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        pendingFunctions_.emplace_back(cb);
    } 
    if(!isInLoopThread()||callingPendingFunctors_){
        wakeup();//因为在doPendingFunctors()中，进行了局部变量的交换操作。所以即使被阻塞在doPendingFunctors()中执行任务，但是functors已经交换，其为空，这样其他线程依然可以往里面加入任务
        //下一次LOOP循环中，因为EPOLL池中已经的wakeupfd已经触发了EPOLLIN，所以直接就进行返回，不会进行阻塞
    }
}

void Eventloop::updateChannel(channel* mp){
    poller_->updateChannel(mp);//epollpoll的update
}

void Eventloop::removeChannel(channel* mp){
    std::cout<<333<<std::endl;
    poller_->removeChannel(mp);
}

bool Eventloop::hasChannel(channel* mp){
    poller_->hasChannel(mp);
}

void Eventloop::wakeup()
{
  uint64_t one=1;
  ssize_t n=write(wakeupfd_,&one,sizeof(one));
  if(n!=sizeof(one))
  {
   LOG_ERROR("EventLoop::wakeup() writes %d bytes instead of 8",static_cast<int>(n));
  }
}//调用wakeup时，监听的wakeupfd就会触发调用handleread(),从而好执行doPendingFunctors（）

/*每一个subreactor都监听一个wakeupfd_,其来进行对主从reactor进行通信*/
void Eventloop::handleRead(){
    uint64_t one=1;
    ssize_t n=read(wakeupfd_,&one,sizeof(one));//未设置阻塞
    if(n!=sizeof(one)){
        LOG_ERROR("EventLoop::handleRead() reads %d bytes instead of 8",static_cast<int>(n));
    }
}

void Eventloop::doPendingFunctors(){
    std::vector<Functor> functors;
    callingPendingFunctors_=true;
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        functors.swap(pendingFunctions_);//交换到局部变量中，防止被延时   
    }
    for(const Functor& functor:functors){
        functor();
    }
   callingPendingFunctors_=false;
}//doPendingFunctors可能调用queueinloop，或其他线程调用，此时有了新任务。等执行完毕后，还要在epoll_wait上阻塞，因为及时回调，所以进行唤醒

TimerId Eventloop::runAt(Timestamp time,Timercallback cb)
{
 return timerQueue_->addTimer(std::move(cb),time,0.0);
}
  TimerId Eventloop::runAfter(double delay ,Timercallback cb)
  {
     Timestamp nowTime(Timestamp::now());//此时刻（当作微秒），延长多少微秒后执行
     return runAt(nowTime.addTime(nowTime,delay),std::move(cb));
    
  }
  TimerId Eventloop::runEvery(double interval,Timercallback cb)
  {
   Timestamp nowTime(Timestamp::now());
   return timerQueue_->addTimer(std::move(cb),nowTime.addTime(nowTime,interval),interval);
  }

  void Eventloop::cancel(TimerId timerid)
  {
    return timerQueue_->cancel(timerid);
  }

  size_t Eventloop::queueSize() const{
     std::unique_lock<std::mutex> locker(m_mutex);
     return pendingFunctions_.size();
  }


/*
poller被eventloop管理，所以channel,Timerqueue等储存自己所属的eventloop，方便调用epoller从而进行事件回调
*/