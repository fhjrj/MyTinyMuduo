#ifndef _ACCEPTOR_
#define _ACCEPTOR_

#include "noncopyable.h"
#include "Eventloop.h"
#include "Inetaddress.h"
#include "Socket.h"
#include "logger.h"
#include <functional>
#include "Channel.h"

//此类主要是listenfd的相关回调和连接函数的进一步封装。
//具体操作封装在socket中
 class Acceptor:public noncopyable
 {
 
 public:
 using NewConnectionCallback=std::function<void(int sockfd,const InetAddress&)>;

    Acceptor(Eventloop*,const InetAddress&,bool);
    ~Acceptor();
    void setNewConnectionCallback(const NewConnectionCallback& cb){//TCPserver中的newConnection
       NewConnectionCallback_=cb;
    }

    bool listening() const{
        return listening_;
    }
    
    void listen();
    private:
    void handleRead_();
    Eventloop* loop_;//用户定义的baseloop_
    Socket AcceptSockfd_;//listenfd
    channel acceptChannel_;//listenfd所属的channel
    NewConnectionCallback  NewConnectionCallback_;
    bool listening_;
 };
 
#endif

//运行在baseloop_中，接受CONNFD连接 再轮训投递给SUBLOOP中