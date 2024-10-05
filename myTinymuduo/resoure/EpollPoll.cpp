#include "EpollPoll.h"
#include "Channel.h"
#include "logger.h"
#include <iostream>
#include <errno.h>
#include <sys/epoll.h>
#include <string.h>
#include <unistd.h>

/*对应的index*/
const int KNew=-1;//channel未加入到poller中
const int KAdded=1;//channel已经加入到poller中
const int KDeleted=2;//channel已经从poller中删除
//记录channel在poller的状态，channel加入Poller中，但是其对任何事情不感兴趣
EPollPoller::EPollPoller(Eventloop* loop)
:Poller(loop),
epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
events_(KInitEventListSize)
{   
    if(epollfd_<0)
   LOG_FATAL("epoll_create_error:%d \n",errno);
}

EPollPoller::~EPollPoller()
{
    ::close(epollfd_);
}

/*每一次epoll_wait调用后，将发生的事件都加入activechannels*/
void EPollPoller::fillActiveChannels(int numEvent,ChannelList* activechannels) const
{ 

    for(int i=0;i<numEvent;i++){
        channel *channel_t = static_cast<channel *>(events_[i].data.ptr);        
        channel_t->set_revents(events_[i].events);
        activechannels->push_back(channel_t); // EventLoop就拿到了它的Poller给它返回的所有发生事件的channel列表了
    }
}

//eventloop->poller poll action
Timestamp EPollPoller::poll(int timeoutMS,ChannelList* activeChannels)
{
    LOG_INFO("fd total count :%lu \n",channels_.size());
    int numEvents=::epoll_wait(epollfd_,&(*events_.begin()),static_cast<int>(events_.size()),timeoutMS);
    //.size() return size_t
    int saveErrno=errno;
    Timestamp now(Timestamp::now());
    if(numEvents>0)
    {
        LOG_INFO("%d events happened\n",numEvents);
        fillActiveChannels(numEvents,activeChannels);//发生了对应的事件
        if(numEvents==events_.size())
        {
            events_.reserve(2*events_.size());
        }
    }
    else if(numEvents==0) //timeout
        { 
           LOG_DEBUG("%s timeout!\n", __FUNCTION__);
        }
        else {
            if(saveErrno!=EINTR){
                errno=saveErrno;//不是系统中断
                LOG_ERROR("EPollPoller::poll()err! \n");
            }
        }
          return now;
    }
  


void EPollPoller::updateChannel(channel* channel)
{
    const int index=channel->index();
    LOG_INFO("func=%s => fd=%d event=%d index=%d \n","updateChannel",channel->fd(),channel->events(),index);
    if(index==KNew||index==KDeleted)/*从未加入或者从poller中删除*/
    {
        if(index==KNew)
        {
            int fd=channel->fd();
            channels_[fd]=channel;
        }else{

        }//index==KDELETED
        
        channel->set_index(KAdded);
        update(EPOLL_CTL_ADD,channel);//注册监听
    }
    else{
        int fd=channel->fd();
        if(channel->isNoneEvent())//任何事件都不感兴趣，调用删除
        {
    update(EPOLL_CTL_DEL,channel);
    channel->set_index(KDeleted);
        }else{
            update(EPOLL_CTL_MOD,channel);//写 读事件
        }
    }    
}


void EPollPoller::update(int events,channel* channel_){
   epoll_event event;
  ::memset(&event,0,sizeof(event));
   int fd=channel_->fd();
   event.events = channel_->events();//返回的是修改事件
    event.data.fd = fd;
    event.data.ptr = channel_;
   if(::epoll_ctl(epollfd_,events,fd,&event)<0)
   {
     if(events==EPOLL_CTL_DEL){
     LOG_ERROR("epoll_ctl error:%d\n",errno);
     }else{
        LOG_FATAL("epoll_ctl add/mod error:%d\n",errno);
     }
   }
}


void EPollPoller::removeChannel(channel* channel)
{   
    int fd=channel->fd();
    int index=channel->index();
    LOG_INFO("func=%s => fd=%d \n","removeChannel()",channel->fd());
    channels_.erase(fd);
    if(index==KAdded)//这里注意 remove是将其从MAP中删除，这里判断是防止MAP中删除，但是EPOLL池中未删除
    {
        update(EPOLL_CTL_DEL,channel);
    }
    channel->set_index(KNew);
}

