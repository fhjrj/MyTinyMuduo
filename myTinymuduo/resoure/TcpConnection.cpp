#include "TcpConnection.h"
#include "logger.h"
#include "Channel.h"
#include <iostream>
#include <errno.h>
#include <functional>
#include <string>
#include <assert.h>
static Eventloop* checkLoopNotNull(Eventloop* Loop){
    if(Loop==nullptr){
        LOG_FATAL("from TcpConnect::TcpConnect the loop is null \n");
    }else{
        return Loop;
    }
}

    TcpConnection::TcpConnection(Eventloop* loop,const std::string& name,int sockfd,const InetAddress& loaclAddr,const InetAddress& peerAddr):
    loop_(checkLoopNotNull(loop)),
    name_(name),
    socket_(new Socket(sockfd)),
    localAddr_(loaclAddr),
    state_(kConnecting),//构造过程中是kConnection
    peerAddr_(peerAddr),
    reading_(true),
    channel_(new channel(loop,sockfd)),
    highWaterMark_(64*1024*1024)
    {
        //设置CHANNEL完成相应事件后的回调函数
        channel_->setreadcallback(std::bind(&TcpConnection::handleRead,this,std::placeholders::_1));
        channel_->setwritecallback(std::bind(&TcpConnection::handlewrite,this));
        channel_->setclosecallback(std::bind(&TcpConnection::handleClose,this));
        channel_->seterrorcallback(std::bind(&TcpConnection::handleError,this));
        LOG_INFO("TcpConnection::ctor[%s] at fd=%d\n", name_.c_str(), sockfd);
        socket_->setKeepalive(true);
    }
   
   TcpConnection::~TcpConnection(){
   LOG_INFO("TcpConnection::dtor[%s] at fd=%d state=%d\n", name_.c_str(), channel_->fd(), (int)state_);
   }


   void TcpConnection::handleRead(Timestamp receiveTime){//触发EPOLL_IN的回调,这里的recviveTime是调用Eventloop::loop()返回的时间
     int savedErro=0;
     ssize_t n=inputBuffer_.readFd(channel_->fd(),&savedErro);//读到Buffer中，进行回调
     if(n>0){
        messageCallback_(shared_from_this(),&inputBuffer_,receiveTime);//执行回调，一般执行的是TcpConnection::send,对消息进行回传
     }else if(n==0){
        handleClose();//对面关闭
     }else{
        errno=savedErro;
        LOG_ERROR("TcpConnection: handleRead: %d\n",errno);
        handleError();
     }
   }
   /*这里的错误回调是针对于read的结果，
   channel中的错误回调是争对监情况的，两者不同
   */

//数据过多的情况下才进行回调
   void TcpConnection::handlewrite(){
     
     if(channel_->isWritingEvent())
     {
        int savedErrno = 0;
        ssize_t n = outputBuffer_.writeFd(channel_->fd(), &savedErrno);
     if(n>0){
        outputBuffer_.retrieve(n);//更新下标
        if(outputBuffer_.readableBytes()==0){//数据已经全部发送过去才进行消除写触发，不然直到最后才进行消除写出发
            channel_->disableWriting();
            if(writeCompleteCallback_)
            {
            loop_->queueInLoop(std::bind(writeCompleteCallback_,shared_from_this()));//数据都发送过去了，然后进行写后事件回调
            }
             if(state_==kDisconnecting)
        {
            shutdownInLoop();//若状态是正在关闭，说明外面不希望再次发送消息，进行SOCKFD的关闭
        }
        }  
     }
   else{
     LOG_DEBUG("TcpConnection::handlewrite\n");
        if(state_==kDisconnecting)//因为又可能在回调之间就调用了shutdown方法
        {
            shutdownInLoop();
        }
   }
   }else{
    LOG_INFO("Connection fd %d is down,no more writing",channel_->fd());
   }
   }

//POller--->channel::closecallback----->TcpConnection::handleClose  对面文件描述符关闭
   void TcpConnection::handleClose(){
  
    LOG_INFO("TcpCoeection::handleClose : the close fd is %d\n",channel_->fd());
    setState(kDisconnected);
    channel_->disableAll();//设置后，channel::update--->eventloop::update--->epollpoll::update--->epoll_ctl::EPOLL_DET,但还未从map中删除
    
    TcpConnectionPtr connPtr(shared_from_this());//这里的智能指针管理的类早就是已经创建好的
    connectionCallback_(connPtr);//执行连接关闭的回调，结合Tcpconnection::connect()使用
    closeCallback_(connPtr);//关闭连接的回调----->Tcpserver::removeConnection
   }

   void TcpConnection::handleError()
   {
     int optval;
     socklen_t optlen=sizeof(optval);
     int err=0;
     if(::getsockopt(channel_->fd(),SOL_SOCKET,SO_ERROR,&optval,&optlen)<0){
     err=errno;
     }else{
        err=optval;
     }
     LOG_ERROR("TCPConnection::handleerror  name %s -SO-ERROR :%d \n",name_.c_str(),err);
   }

  void TcpConnection::send(const std::string& messgae){//用户调用
    if(state_==kConnected){
      if(loop_->isInLoopThread()){
        sendInLoop(messgae.c_str(),messgae.size());
      }
      else{
        loop_->runInLoop(std::bind(&TcpConnection::sendInLoop,this,messgae.c_str(),messgae.size()));
      }
    }
  }

  void TcpConnection::sendInLoop(const void* message,size_t len){
     ssize_t nwrote=0;
     size_t remaining=len;
     bool faultError=false;
     if(state_==kDisconnected){//调用了SHUTDOWN函数，不能发送
      LOG_ERROR("disconnected,give up writing");
     }
//channel第一次写数据的情况下，肯定未设置epollin和BUFFER中没有数据，或者是已经执行过写回调的情况下
     if(!channel_->isWritingEvent()&&outputBuffer_.readableBytes()==0){ 
      nwrote=::write(channel_->fd(),message,len);
      if(nwrote>=0){
        remaining=len-nwrote;
        if(remaining==0&&writeCompleteCallback_){
          loop_->queueInLoop(std::bind(writeCompleteCallback_,shared_from_this()));//这里的写回调已经提前注册过了
          //这里的数据全部发送完毕了，不用再进行设置EPOLLOUT事件了，直接进行写操作完毕的回调函数
        }
      }else{
        nwrote=0;
        if(errno!=EWOULDBLOCK){
       LOG_ERROR("TcpConnection::sendInLoop");
       if(errno==EPIPE||errno==ECONNRESET){
        faultError=true;
       }
        }
      }
     }

      if(!faultError&&remaining>0){//此情况是数据太多，当前这一次WRITE未将数据都发送出去，剩余的数据要保存在BUFFER中，给CHANNEL注册EPOLLOUT,进行回调执行handlerwrite---->hanlerwrite处理的是还未发送完毕的数据，相当于发送了两次
      //remaining>0的情况才会注册EPOLLOUT,如果消息一次性传出，则不用进行EPOLLOUT设置，不用触发写回调
       size_t oldlen=outputBuffer_.readableBytes();
       if(oldlen+remaining>=highWaterMark_&&
       oldlen<highWaterMark_){
        loop_->queueInLoop(
          std::bind(highWaterMarkCallback_,shared_from_this(),oldlen+remaining)
        );
       }
       outputBuffer_.append((char*)message+nwrote,remaining);//内容进行追加
       if(!channel_->isWritingEvent()){//设置写准备，进行触发写回调
        channel_->enableWriting();
       }
      }
  }

 void TcpConnection::connectEstablished(){
  assert(state_=kConnecting);
  setState(kConnected);
  channel_->tie(shared_from_this());//进行弱智能指针的关联
  channel_->enableReading();//注册可读事件，逐步被监听回调
  connectionCallback_(shared_from_this());
 }

 void TcpConnection::connectDestoryed(){
  if(state_==kConnected){
    setState(kDisconnected);
    channel_->disableAll();//EPOLL监听中删除
    connectionCallback_(shared_from_this());//
  }
  channel_->remove();//MAP中删除
 }

 void TcpConnection::shutdown(){//外面调用
     if(state_==kConnected){//只有在连接上以后，才进行关闭
      setState(kDisconnecting);//调用SHUTDOWN方法，这里置为kdisconnecting
      loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop,this));
     }
 }

void TcpConnection::shutdownInLoop(){
  if(!channel_->isWritingEvent()){//数据都发送完成，这里对应TcpConnection::handlewrite()中的消除写的操作
 socket_->shutdownwrite();
  }
}
/*执行shutdown方法，将写端关闭，然后在Tcpserver执行handleclose*/
//在数据过多的情况下，才进行设置写回调，由于不是ET，所以会一直触发