#include "logger.h"
#include <string>
#include <iostream>
#include "Timestamp.h"

std::shared_ptr<Logger> Logger::instance(){
    if(logerinstance.get()){
        return logerinstance;
    }else{
        logerinstance=std::shared_ptr<Logger>(new Logger);//不使用std::make_shared
        return logerinstance;
    }
}

void Logger::getLoglevel(int level){
    Loglevel_=level;
}

void Logger::log(std::string msg){
    std::string cs;
   switch (Loglevel_)
   {
     case INFO:
    cs+="[INFO] ";
     break;
      case ERROR:
    cs+="[ERROR] ";
     break;
     case FATAL:
    cs+="[TATAL] ";
     break;
     case DEBUG:
    cs+="[DEBUG] ";
     break;
     default:
     break;
   }
   if(m_async_log==1){
  std::string totollog=Timestamp::now().Tostring();
  cs+=totollog;
  cs+=" : ";
  cs+=msg;
  m_queue.push(cs);
  m_cond.notify_one();
   }else{
    cs+=Timestamp::now().Tostring();
    cs+=" : ";
    cs+=msg;
    std::cout<<cs<<std::endl;
   }
}

void Logger::asynclog(){
    while(1){
        std::unique_lock<std::mutex> locker(m_mutex);
       while(m_queue.empty())
       {
        m_cond.wait(locker);
       }
        std::string mg=m_queue.front();
        m_queue.pop();
        std::cout<<mg<<std::endl;
    }
}

void Logger::start(){
    if(m_async_log==0){
        return ;
    }else{
        std::thread m1([this](){
        this->asynclog();
        });
        std::thread m2([this](){
        this->asynclog();
        });
        m1.detach();
        m2.detach();
    }
}

std::shared_ptr<Logger> Logger::logerinstance=nullptr;


