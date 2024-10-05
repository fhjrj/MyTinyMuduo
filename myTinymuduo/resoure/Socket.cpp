#include "Socket.h"
#include "logger.h"
#include <iostream>
#include <strings.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

  Socket::~Socket(){
    close(sockfd_);
  }

  void Socket::bindAddress(const InetAddress&address_){//外面传进来已经初始化好了
    if(0!= ::bind(sockfd_,( struct sockaddr*)address_.getsockaddr(),sizeof(sockaddr_in)))
    {
        LOG_FATAL("bind sockfd %d fail \n",sockfd_);
    }
  }

  void Socket::listen(){
    if(0!=::listen(sockfd_,1024)){
        LOG_FATAL("listen sockfd %d fail \n",sockfd_);
    }
  }

  int Socket::accept(InetAddress* address){//外面传过来的 进行绑定返回
    sockaddr_in listen;
    socklen_t size_=sizeof(listen);
    ::bzero(&listen,sizeof(listen));
    int tmp=::accept4(sockfd_,(struct sockaddr*)&listen,&size_,SOCK_NONBLOCK|SOCK_CLOEXEC);//设置非阻塞
    if(tmp>=0)
{
 address->setSockAddr(listen);//储存对面的信息
}
  return tmp;
  }

  void Socket::shutdownwrite(){
    if(::shutdown(sockfd_,SHUT_WR)<0){
        LOG_ERROR("shutdown() error");
    }
  }


  void Socket::setTcpNoDelay(bool a){
    int opt=a?1:0;
    ::setsockopt(sockfd_,IPPROTO_TCP,TCP_NODELAY,(void*)&opt,sizeof(opt));
  }

  void Socket::setReuseAddr(bool a){
     int opt=a?1:0;
      ::setsockopt(sockfd_,SOL_SOCKET,SO_REUSEADDR,(void*)&opt,sizeof(opt));
  }

  void Socket::setKeepalive(bool a){
      int opt=a?1:0;
      ::setsockopt(sockfd_,SOL_SOCKET,SO_KEEPALIVE,(void*)&opt,sizeof(opt));
  }

 void Socket::setReusePoot(bool a){
    int opt=a?1:0;
      ::setsockopt(sockfd_,SOL_SOCKET,SO_REUSEPORT,(void*)&opt,sizeof(opt));  
 }