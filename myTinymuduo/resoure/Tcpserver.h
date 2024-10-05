#ifndef _TCP_SERVER_
#define _TCP_SERVER_

 #include "noncopyable.h"
 #include "Eventloop.h"
 #include "Acceptor.h"
 #include "Inetaddress.h"
 #include "TcpConnection.h"
 #include "EventLoopThreadPool.h"
 #include "Callbacks.h"
 #include "Buffer.h"
 #include <iostream>
 #include <string>
 #include <memory>
 #include <functional>
 #include <atomic>
 #include <unordered_map>

class TcpServer: public noncopyable
{
 public:
 using ThreadInetCallback=std::function<void(Eventloop*)>;

 enum Option{
kNoReusePort,
    kReusePort,
 };//端口是否可重用

 TcpServer(Eventloop* loop,const InetAddress& listenAddr,const std::string nameArg,Option option=kNoReusePort);
 ~TcpServer();//TCPserver使用时都先绑定一个接受端，专门接受新客户连接

 const std::string& ipPort_() const{
    return ipPort;
 }
 const std::string& name() const{
   return  name_;
 }
 Eventloop* getLoop() const {return loop_;};

 void setThreadNum(int numThreads);//设置底层SUBLOOP的个数

  void start();
  //回调设置
    void setConnectionCallback(const ConnectionCallback& cb)
  {  
     connectionCallback_ = cb;
      }
   void setMessageCallback(const MessageCallback& cb)
  { messageCallback_ = cb; }
   void setWriteCompleteCallback(const WriteCompleteCallback& cb)
  { writeCompleteCallback_ = cb; }
  void setThreadInitCallback(const ThreadInetCallback& cb){
    threadInetCallback_=cb;
 }

 private:
 void newConnection(int sockfd,const InetAddress& peerAddr);//加入新的连接
 void removeConnection(const TcpConnectionPtr& conn);
 void removeConnectionInLoop(const TcpConnectionPtr& conn);

 using ConnectionMap=std::unordered_map<std::string,TcpConnectionPtr>;//索引寻找每个客户对应的Tcpconnection
 Eventloop* loop_;//baseloop;
 const std::string ipPort;
 const std::string name_;
 std::unique_ptr<Acceptor> acceptor_;//acceptor作用于baseloop，接受连接
 std::shared_ptr<EventLoopThreadPoll> threadPoll_;//事件循环线程池

 //还未开始前，先进行设置了一些回调函数
 ConnectionCallback connectionCallback_;//连接用户之后的回调
 MessageCallback  messageCallback_;//数据存入BUFFER中的回调 
WriteCompleteCallback  writeCompleteCallback_;//完成写的回调 --->EPOLL中对应更改MOD
ThreadInetCallback threadInetCallback_;//LOOP线程初始化的回调
/*EVENTLOOP进行回调*/

std::atomic<int> started;
int nextConnId_;
ConnectionMap connections_;//保存所有的连接变量
};

#endif