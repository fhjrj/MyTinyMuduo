#include "HttpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "Http_analyse.h"
#include <iostream>
#include <memory>

void defaultHttpCallback(const HttpRequest&,HttpRespne* resp)
{
    resp->setStatusCode(HttpRespne::HttpStatusCode::K404NotFound);
    resp->setStatusMessage("Not Found");
    resp->closeConnection(true);
}


HttpServer::HttpServer(Eventloop* loop,const InetAddress& listenAddr,const std::string name,TcpServer::Option option):
server_(loop,listenAddr,name,option),
httpCallback_(defaultHttpCallback)
{
    server_.setConnectionCallback(std::bind(&HttpServer::onConnection,this,std::placeholders::_1));
    server_.setMessageCallback(std::bind(&HttpServer::onMessage,this,
    std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
    server_.setThreadNum(4);
}
//占位符是TcpConnection中已经既定要穿的参数
void HttpServer::start()
{
     std::cout<< "HttpServer[" << server_.name()
    << "] starts listening on " << server_.ipPort_()<<std::endl;
  server_.start();
}

void HttpServer::onConnection(const TcpConnectionPtr& ptr){
    if(ptr->connected())
    {
        std::cout<<"new connection got"<<std::endl;
    }
    else{
        std::cout<<"connection closed"<<std::endl;
    }
}


void HttpServer::onMessage(const TcpConnectionPtr& coon,Buffer* buf,Timestamp recvtime)//TcpConnection中的buffer
{
   std::unique_ptr<HttpAnalyse>  yser(new HttpAnalyse());
   
   if(!yser->parseRequest(buf,recvtime))//消息头分析错误
   {

     std::string p="parseRequest failed\n";
     coon->send(p);
     coon->send("HTTP/1.1 400 Bad Request\r\n\r\n");
     coon->shutdown();
   }

   if(yser->isover())
   {
     std::string p="parseRequest success!\n";
     OnRequest(coon,yser->getrequset());//进行消息组装
     yser->reset();
   }//组织回调进行回复
}

void HttpServer::OnRequest(const TcpConnectionPtr& conn,const HttpRequest& req)
{
   const std::string& connection=req.getHeader("Connection");
   bool close=((connection=="close")||
   (req.Getversion()==HttpRequest::VERSION::HTTP10&&connection!="Keep-alive"));
   HttpRespne respone(close);
   httpCallback_(req,&respone);//消息组装，情况在respone中
   Buffer buf;
   respone.appendToBuffer(&buf);
  const std::string message(buf.to_string());
  if(message.size()!=0)
    conn->send(message);

    if(respone.getclose()){
      conn->shutdown();
    }
}
//std部分用日志代替更好





