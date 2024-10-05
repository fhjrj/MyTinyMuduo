#include "Poller.h"
#include "Channel.h"

Poller::Poller(Eventloop* loop_):ownerLoop_(loop_) {}//eventloop中调用 传入this

bool Poller::hasChannel(channel* channell) const
{
 auto it=channels_.find(channell->fd());
 return it!=channels_.end()&&it->second==channell;
}

//epollpoll-------poller