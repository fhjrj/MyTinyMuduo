#ifndef _EPOLLPOLLER_
#define _EPOLLPOLLER_

#include "Poller.h"
#include <sys/epoll.h>
#include <vector>
#include "Timestamp.h"
#include "Channel.h"

class EPollPoller :public Poller
{
 public:
 EPollPoller(Eventloop* loop);
 ~EPollPoller() override;

 Timestamp poll(int timeoutMs,ChannelList* activeChannels) override;
 void updateChannel(channel* channell) override;
 void removeChannel(channel* channell) override;

 private:
 static const int KInitEventListSize=16;
 
 using EventList=std::vector<struct epoll_event>;

 int epollfd_;
 EventList events_;

 void fillActiveChannels(int numEvents,ChannelList* activeChannels) const;//填写活跃连接

 void update(int operaton,channel* channel__);//等效于EPOLL_MOD,进行更改事件
};
//eventfd在epollpoll中，而eventloop又管理自己的poll/epollpoll

#endif

