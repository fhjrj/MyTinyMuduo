#ifndef HTTP_SERVER
#define HTTP_SERVER

#include <functional>
#include "Tcpserver.h"
#include "Callbacks.h"
#include "noncopyable.h"
class HttpRespne;
class HttpRequest;
class HttpServer :public noncopyable
{
   public:
   using HttpCallback=std::function<void(const HttpRequest&,HttpRespne*)> ;

   HttpServer(Eventloop* loop,const InetAddress& listenAddr,const std::string name,TcpServer::Option option=TcpServer::kNoReusePort);//baseloop
   void start();

   void setThreadNum(int numThreads)
   {
    server_.setThreadNum(numThreads);
   }
  
  void setHttpCallback(const HttpCallback& cb)
  {
    httpCallback_ = cb;
  }

  Eventloop* getLoop() const { return server_.getLoop(); }

   private:
   void onConnection(const TcpConnectionPtr& conn);
   void onMessage(const TcpConnectionPtr&conn,Buffer* buffer,Timestamp revetime);//接受消息后，进行消息分析并且发送
   void OnRequest(const TcpConnectionPtr& ,const HttpRequest&);

   TcpServer server_;
   HttpCallback httpCallback_; 
};

#endif