
#include <string>
#include <iostream>
#include "Tcpserver.h"
#include "logger.h"
#include "Eventloop.h"


class EchoServer
{
public:
    EchoServer(Eventloop *loop, const InetAddress &addr, const std::string &name)//先提前绑定一个
        : server_(loop, addr, name)
        , loop_(loop)
    {
        // 注册回调函数 TCPSERVER---->TCPCONNECTION-->CHANNEL  
        server_.setConnectionCallback(
            std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
        server_.setMessageCallback(
            std::bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        // 设置合适的subloop线程数量
        server_.setThreadNum(3);
    }
    void start()
    {
        server_.start();//accpet开始监听
    }

private:
    // 连接建立或断开的回调函数
    void onConnection(const TcpConnectionPtr &conn)   
    {
        if (conn->connected())
        {
            LOG_INFO("Connection UP : %s", conn->peerAddr().get_ip_port().c_str());
        }
        else
        {
            LOG_INFO("Connection DOWN : %s", conn->peerAddr().get_ip_port().c_str());
        }
    }

    // 可读事件回调
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
    {
        std::string msg = buf->retrieveAllAsString();
        conn->send(msg);
        // conn->shutdown();   // 关闭写端 底层响应EPOLLHUP => 执行closeCallback_
    }

    Eventloop *loop_;
    TcpServer server_;
};

int main() {
    Eventloop loop;
    InetAddress addr(8000);
    EchoServer server(&loop, addr, "EchoServer");
    server.start();
    loop.loop();
    return 0;
}
/*
对面传来消息，触发读回调，消息储存在buffer中，进行调用用户自己的写回调，即onMessage() 其中调用send()函数，若数据过多，会触发写事件，进行写回调，即将多余数据存在
writebuffer中发送
*/




 /*#include "Eventloop.h"
#include "EventLoopThread.h"
int cnt = 0;
Eventloop* g_loop;

void printTid()
{
  printf("pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
  printf("now %s\n", Timestamp::now().Tostring().c_str());
}

void print(const char* msg)
{
  printf("msg %s %s\n", Timestamp::now().Tostring().c_str(), msg);
  if (++cnt == 20)
  {
    g_loop->quit();
  }
}

void cancel(TimerId timer)
{
  g_loop->cancel(timer);
  printf("cancelled at %s\n", Timestamp::now().Tostring().c_str());
}

int main()
{
  printTid();
  sleep(1);
  {
    Eventloop loop;//一个eventloop
    g_loop = &loop;

    print("main");
    loop.runAfter(1, std::bind(print, "once1"));
    loop.runAfter(1.5, std::bind(print, "once1.5"));
    loop.runAfter(2.5, std::bind(print, "once2.5"));
    loop.runAfter(3.5, std::bind(print, "once3.5"));
    TimerId t45 = loop.runAfter(4.5, std::bind(print, "once4.5"));
    loop.runAfter(4.2, std::bind(cancel, t45));
    loop.runAfter(4.8, std::bind(cancel, t45));
    loop.runEvery(2, std::bind(print, "every2"));
    TimerId t3 = loop.runEvery(3, std::bind(print, "every3"));
    loop.runAfter(9.001, std::bind(cancel, t3));

    loop.loop();
    print("main loop exits");//回调不要想复杂 开启loop时才会进行监听，即使超过也会进行更新
  }
  sleep(1);
  {
    EventLoopThread loopThread;
    Eventloop* loop = loopThread.startloop();
    loop->runAfter(2, printTid);
    sleep(3);
    print("thread loop exits");
  }
}
*/
    
