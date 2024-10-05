#include "Channel.h"
#include "logger.h"
#include <iostream>
#include <sys/epoll.h>
#include <functional>

const int channel::KNoneEvent=0;
const int channel::KReadEvent=EPOLLIN|EPOLLPRI;//有紧急数据可读，非EPOLLET
const int channel::KWriteEvent=EPOLLOUT;

channel::channel(Eventloop* loop,int fd):
 loop_(loop),
  fd_(fd),
  events_(0),
  revents_(0),
  index_(-1),
  tied_(false)
  {}

  channel::~channel()
  {}

void channel::set_revents(int rec)
{
  revents_=rec;
}

/*原muduo库中都是进行断言的，判断在析构时，是否还有任务回调，在析构时，必须先移除监听文件描述符，再进行析构*/
/*所以在析构时，必须是调用remove()后，在进行调用析构函数*/
/*还要看析构时，是否是调用所属线程进行析构*/

  void channel::tie(const std::shared_ptr<void>& obj){
    tie_=obj;
    tied_=true;
  }

/*epoll被封装在eventloop中，调用改变事件*/
// channel update/remove->eventloop update/remove ->poll update/remove ->epoll_ctl
  void channel::update()
  { 
     loop_->updateChannel(this);
  }

  void channel::remove(){
   // if(loop_==nullptr) return
     loop_->removeChannel(this);
  }

//调用handleEvent之前，一般会先调用tie，进行监听，判断对象是否消失。一个Tcpconnection新连接创建的时候
  void channel::handleEvent(Timestamp receiveTime)
  {
   if (tied_)
    {
        std::shared_ptr<void> guard = tie_.lock();
        if (guard)
        {
            handleEventWithGuard(receiveTime);
        }
        // 如果提升失败了 就不做任何处理 说明Channel的TcpConnection对象已经不存在了
    }
    else
    {
        handleEventWithGuard(receiveTime);
    }
  }

//eventloop中包含的是std::vector<channel*> 类，用于事件循环，相当于epoll_event数组
  void channel::handleEventWithGuard(Timestamp reveiveTime){
      //依次进行判断 不是else if这种。在此里面对事件进行分类，不同于webserver中的分类。
       LOG_INFO("channel handleEvent revents: %d\n",revents_);

        if((revents_&EPOLLHUP)&&!(revents_&EPOLLIN)){
        
         if(closecallback_) closecallback_();
        }

        if(revents_&(EPOLLERR))
        {
            if(errorcallback_) errorcallback_();
        }

        if(revents_&(EPOLLIN|EPOLLPRI))
        {
            if(readcallback_) readcallback_(reveiveTime);
        }

        if(revents_&(EPOLLOUT))
        {
               if(writecallback_) writecallback_();
        }

  }//任务回调在TCPCONNECTION中

/*设置多个监听事件是| 查看是否该事件用&*/