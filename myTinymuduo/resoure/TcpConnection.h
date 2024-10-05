#ifndef TCPSERVER
#define TCPSERVER

#include "noncopyable.h"
#include "Eventloop.h"
#include "Socket.h"
#include "Channel.h"
#include "Buffer.h"
#include "Callbacks.h"
#include "Timestamp.h"
#include "Inetaddress.h"
#include <sys/types.h>
#include <string>
#include <memory>
#include <atomic>
class TcpConnection :public noncopyable,public std::enable_shared_from_this<TcpConnection>//储存连接用户双方的信息
{
   public:
  TcpConnection(Eventloop* loop,const std::string& name,int sockfd,const InetAddress& loaclAddr,const InetAddress& peerAddr);
  ~TcpConnection();

  Eventloop* getloop() const {return loop_;}
  const std::string name() const {return name_;}
  const InetAddress&  localAddr() const {return localAddr_;}
  const InetAddress& peerAddr() const {return peerAddr_;}
  bool connected() const {return state_==kConnected;}
  void shutdown();//关闭当前连接
  //回调函数设置
  void setConnectionCallback(const ConnectionCallback& cb)
  { connectionCallback_ = cb; }
   void setMessageCallback(const MessageCallback& cb)//信息回调，负责信息的发送和接受
  { messageCallback_ = cb; }
   void setWriteCompleteCallback(const WriteCompleteCallback& cb)
  { writeCompleteCallback_ = cb; }
  void setHighWaterMarkCallback(const HighWaterMarkCallback& cb,size_t highWaterMark__){
    highWaterMarkCallback_=cb;
    highWaterMark_=highWaterMark__;
  }
  void setClosecallback(const CloseCallback& cb)
  {closeCallback_=cb;}

  void connectEstablished();//连接建立
  void connectDestoryed();//销毁连接
   void send(const std::string& message);
 private:
 //进行事件的回调
 void handleRead(Timestamp reveiveTime);
 void handlewrite();
 void handleClose();
 void handleError();

 void sendInLoop(const void* message,size_t len);
 void shutdownInLoop();
 enum StateE{
 kDisconnected,
 kConnecting,
 kConnected,
 kDisconnecting
 };
 void setState(StateE state){state_=state;}
  Eventloop* loop_;//subloop
  std::atomic<int> state_;
  bool reading_;
  std::unique_ptr<Socket> socket_;//连接对面的sockfd
  std::unique_ptr<channel> channel_;//对上面sockfd事件进行的封装
  const std::string name_;
  const InetAddress localAddr_;
  const InetAddress peerAddr_;

ConnectionCallback connectionCallback_;//连接用户之后的回调
MessageCallback  messageCallback_;//数据存入BUFFER中的回调 
WriteCompleteCallback  writeCompleteCallback_;//完成写的回调 --->EPOLL中对应更改MOD
HighWaterMarkCallback highWaterMarkCallback_;
CloseCallback closeCallback_;

size_t highWaterMark_;
Buffer inputBuffer_;
Buffer outputBuffer_;
};
/*

tcpserver->acceptor->通过ACCEPTER拿到connfd,
tcpconnection设置回调->channel->poller-->channel进行回调操作
TcpConnection在SUBLOOP中的
*/

#endif