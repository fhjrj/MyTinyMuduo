#ifndef _POLLER_H_
#define _POLLER_H_

#include "noncopyable.h"
#include <vector>
#include <unordered_map>
#include "Timestamp.h"
//事件分发器，I/O多路复用模块

class channel;
class Eventloop;

class Poller
{
public:
 using ChannelList=std::vector<channel*>;
 Poller(Eventloop* tp);//该Poller所属的eventloop
 virtual  ~Poller()=default;

//给所有I/O复用调用统一的接口
 virtual Timestamp poll(int timeoutMS,ChannelList* activelist)=0;//等效于epoll_wait
 virtual void updateChannel(channel* channell)=0;//等效EPOLL_MOD
 virtual void removeChannel(channel* channell)=0;//等效EPOLL_CTL

 /*判断channel是否在当前poller当中*/
 bool hasChannel(channel* channel__) const;
 /*此接口可以获得默认的I/O多路复用的具体实现,但因为这是抽象基类，确要返回子类的具体实现，唯一方法是单独定义一个文件进行实现*/
 static Poller* newDefaultPoller(Eventloop* loop);

protected:
 using ChannelMap=std::unordered_map<int,channel*> ;//sockfd<--->channel*,记录当前Poller管理的channel
 ChannelMap channels_;

private:
Eventloop* ownerLoop_;

};

#endif


