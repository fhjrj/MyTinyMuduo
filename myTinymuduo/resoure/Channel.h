#ifndef _CHANNEL_
#define _CHANNEL_

#include "noncopyable.h"
#include "Timestamp.h"
#include "Eventloop.h"
#include <functional>
#include <memory>
/*封装事件类,绑定监听事件类，sockfd,绑定返回具体事件*/

class channel :public noncopyable
{
public:
 int revents_;//返回发生事件
 using EventCallback=std::function<void()> ;
 using ReadEventCallback=std::function<void(Timestamp)>;

channel(Eventloop* loop,int fd);//套接字所属的CHANNEL
~channel();

/*通知之后，进行回调函数*/
void handleEvent(Timestamp receveTime);

/*回调函数设置*/
void setreadcallback(ReadEventCallback cb){
    readcallback_=std::move(cb);
}

void setwritecallback(EventCallback cb){
   writecallback_=std::move(cb);
}

void setclosecallback(EventCallback cb){
    closecallback_=std::move(cb);
}

void seterrorcallback(EventCallback cb){
    errorcallback_=std::move(cb);
}

/*防止channel被移除时，还在进行回调*/
void tie(const std::shared_ptr<void>&);

int fd() const{
    return fd_;
}

int events() const{
    return events_;
}

void set_revents(int revt);


/*设置FD对应的监听事件位*/
void enableReading(){events_ |=KReadEvent;update();}
void disableReading(){events_ &=~KReadEvent;update();}
void enableWriting(){events_ |=KWriteEvent;update();}
void disableWriting(){events_ &=~KWriteEvent;update();}
void disableAll(){events_ =KNoneEvent;update();}
/*查询事件状态*/
bool isNoneEvent() const {return events_ ==KNoneEvent;}
bool isWritingEvent() const {return events_ &KWriteEvent;}
bool isReadingEvent() const {return events_ &KReadEvent;}

int index(){return index_;}
void set_index(int idx){index_=idx;}

/*get eventloop*/
Eventloop* ownerLoop() {return loop_;}

void remove();

/*事件分类*/
static const int KNoneEvent;
static const int KReadEvent;
static const int KWriteEvent;
void handleEventWithGuard(Timestamp receiveTime);
void update();

private:
const int fd_;//唤醒的wakeupfd_,其唤醒的subreactor
int events_;//绑定事件
int index_;
Eventloop* loop_;//事件循环
std::weak_ptr<void> tie_;
bool tied_;

/*回调函数*/
ReadEventCallback readcallback_;
EventCallback writecallback_;
EventCallback closecallback_;
EventCallback errorcallback_;
};

#endif

/*每个线程都有一个eventlop,每个线程进行分开管理，而不是总的一个epoll池*/