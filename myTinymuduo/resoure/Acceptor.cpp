#include "Acceptor.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <errno.h>
 static int createNoblocking(){
 int sockfd=::socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK|SOCK_CLOEXEC,IPPROTO_TCP);
 if(sockfd<0){
    LOG_FATAL(" listen socket create err:%d \n",errno);
 }
 return sockfd;
 }

//传进来的address中的sockaddr_in已经初始化了
Acceptor::Acceptor(Eventloop* loop,const InetAddress& address,bool requset):loop_(loop),listening_(false),
AcceptSockfd_(createNoblocking()),
acceptChannel_(loop_,AcceptSockfd_.fd()){
   AcceptSockfd_.setReuseAddr(true);
   AcceptSockfd_.setReusePoot(true);
   AcceptSockfd_.bindAddress(address);//套接字的绑定
   acceptChannel_.setreadcallback(std::bind(&Acceptor::handleRead_,this));//进行绑定监听事件发生后的事件回调
}

Acceptor::~Acceptor(){
   acceptChannel_.disableAll();//清除
   acceptChannel_.remove();//--->eventloop.remove---->epollpoll.remove
}

void Acceptor::listen(){
   listening_=true;
   AcceptSockfd_.listen();
   acceptChannel_.enableReading();//channel::update------>eventloop::update------>epoll_ctl
}

//listenfd有新用户连接了，所以会进行此函数调用
void Acceptor::handleRead_(){
   InetAddress peerAddr;
   int connfd=AcceptSockfd_.accept(&peerAddr);//获得连接对面的文件文件描述符和信息
   if(connfd>=0){
      //进行回调，事件监听 connfd->channel->sunloop
      if(NewConnectionCallback_){
         NewConnectionCallback_(connfd,peerAddr);//Tcpserver::newconnection--->分发给SUBLOOP
      }else{
         ::close(connfd);
      }
   }else{
      LOG_ERROR("%s:%s:%d accept err:%d\n", __FILE__, __FUNCTION__, __LINE__, errno);
        if (errno == EMFILE)
        {
            LOG_ERROR("%s:%s:%d sockfd reached limit\n", __FILE__, __FUNCTION__, __LINE__);
        }
   }
}