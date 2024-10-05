#include "Tcpserver.h"
#include "logger.h"
#include <string.h>
#include <assert.h>
#include <functional>
static Eventloop* checkLoopNotNull(Eventloop* Loop){
    if(Loop==nullptr){
        LOG_FATAL("from TcpServer::TcpServer the loop is null \n");
    }else{
        return Loop;
    }
}

TcpServer::TcpServer(Eventloop* loop,const InetAddress& listenaddr,const std::string nameArg,Option option):
       loop_(checkLoopNotNull(loop)),
    ipPort(listenaddr.get_ip_port()),
    name_(nameArg),
    acceptor_(new Acceptor(loop,listenaddr,option==kReusePort)),//创建acceptor,其对应的channel进行绑定进入到baseloop中,调用listen时才会开始监听
    threadPoll_(new EventLoopThreadPoll(loop,name_)),
    connectionCallback_(),
    messageCallback_(),
    nextConnId_(1),
    started(0){
        acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection,this,
        std::placeholders::_1,std::placeholders::_2));//绑定此函数 当有新用户连接时，执行此连接回调
    }

TcpServer::~TcpServer(){
    LOG_DEBUG("TcpServer::~TcpServer  %s  destructing",name_.c_str());
    for(auto& item:connections_){
        TcpConnectionPtr coon(item.second);//局部对象，出右括号自动析构
        item.second.reset();
        coon->getloop()->runInLoop(
            std::bind(&TcpConnection::connectDestoryed,coon)
        );
    }
}
     //有一个新的客户端连接，accepter会执行这个回调操作
    void TcpServer::newConnection(int sockfd,const InetAddress& newaddr ){
        //newaddr 对端的IP
         Eventloop* ioloop=threadPoll_->getNextLoop();
         char buf[64]={0};
         snprintf(buf,sizeof(buf),"-%s#%d",ipPort.c_str(),nextConnId_);
         ++nextConnId_;
         std::string connName=name_+buf;
         LOG_INFO("TcpServer::newConnection [%s] - newconnection [%s] from %s \n",
         name_.c_str(),connName.c_str(),newaddr.get_ip_port().c_str());
         sockaddr_in localaddr;
         ::memset(&localaddr, 0, sizeof(localaddr));
         socklen_t connsize=sizeof(localaddr);
         if(::getsockname(sockfd,(struct sockaddr*)&localaddr,&connsize)<0){
            LOG_ERROR("sockets::getsockname");
         }
         InetAddress localAddr(localaddr);//我端

         //创建TcpConnection连接对象，这时连接的connfd加入自己对应的loop中，设置回调
         TcpConnectionPtr conn(new TcpConnection(ioloop,connName,sockfd,localAddr,newaddr));
         connections_[connName]=conn;
         //下面的回调都是用户设置给TCPSERVER---->TCPCONNECTION-->CHANNEL  
         conn->setConnectionCallback(connectionCallback_);
         conn->setMessageCallback(messageCallback_);
         conn->setWriteCompleteCallback(writeCompleteCallback_);
         conn->setClosecallback(std::bind(&TcpServer::removeConnection,this,std::placeholders::_1));
         ioloop->runInLoop(std::bind(&TcpConnection::connectEstablished,conn));//建立联系，从而将connfd至于监听状态，
    }
//为什么std::bind()绑定的是智能指针而不是std::shared_ptr::get? 因为这里的智能指针是局部变量
//如果传入std::shared_ptr::get，在回调设置完毕后，局部变量回收，智能指针进行回收。后面执行回调方法时，内存已经没了，却调用函数，这时不合法的
//传递智能指针，引用计数+1，一共为2，绑定完成，引用计数变为1，不会被析构。
 
     void TcpServer::setThreadNum(int Threadnumber_){
        threadPoll_->setThreadNum(Threadnumber_);
     }

     void TcpServer::start(){
       if(started++==0){//防止被start多次
       threadPoll_->start(threadInetCallback_);//启动底层线程池
       loop_->runInLoop(std::bind(&Acceptor::listen,acceptor_.get()));//进行监听
       }
     }

/*MAINLOOP中进行监听accepter,此函数进行删除对应的客户端，加入到其对应所属的SUBLOOP中去执行*/
     void TcpServer::removeConnection(const TcpConnectionPtr& tmp){
        loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop,this,tmp));
     }

     void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& cc){
        LOG_INFO("TcpServer::removeConnectionLoop-----the name is__ %s\n",cc->name().c_str());
        size_t n=connections_.erase(cc->name());//MAP中删除对应的监听Tcp;
        (void)n;
        assert(n==1);
        Eventloop* ioloop=cc->getloop();//取出该客户端所属的SUBLOOP
        ioloop->runInLoop(std::bind(&TcpConnection::connectDestoryed,cc));//loop管理的MAP中删除
     }
     //TcpConnection管理的是客户端的一些操作，比如销毁等等
     //std::bind()中绑定的函数中，调用的是自己类中的函数，就要传参类变量的地址，这里就是this
     //std::bind()绑定指针时，如果在一个类中绑定另一个类的成员变量时，需要另一个类对象的指针，可以传裸指针，也可以传智能指针。
     //传入智能智能时，若是std::shared_ptr<>类，则会引用计数+1

     /*
     初始化,acceptor设置连接成功的回调，同时acceptor设置触发新用户连接时的回调，后者执行过程中，执行前者
     TCPserver中设置的回调函数中TcpConenctiONPtr参数是用来当TcpConnetion进行回调时，回调操作中又有提前设置的用户操作，而用可以通过
     指针来进行执行TcpCONNETION中的其他函数
     */


