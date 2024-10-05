#ifndef _LOG_
#define _LOG_

#include <memory>
#include <string>
#include <thread>
#include <queue>
#include <condition_variable>
#include "noncopyable.h"

static std::mutex mm_mutex;
enum Logevent
{
    INFO,
    ERROR,
    FATAL,
    DEBUG,
};


class Logger :public noncopyable
{
    public:
    static std::shared_ptr<Logger> instance();
    void getLoglevel(int);//获得日志级别
    void log(std::string msg);
    static std::shared_ptr<Logger> logerinstance;
    void init(int m_async_log_){
        m_async_log=m_async_log_;
        start();
    }
    void asynclog();
    void start();
    private:
    std::mutex m_mutex;
    std::condition_variable m_cond;
    int Loglevel_;
    int m_async_log;
    std::queue<std::string> m_queue;
    Logger(){}
  
};


#define LOG_INFO(logmsg,...) \
  do \
  { \
    std::unique_lock<std::mutex> locker(mm_mutex); \
    std::shared_ptr<Logger> loginst=Logger::instance(); \
    loginst->getLoglevel(INFO); \
    char buf[1024]={0}; \
    snprintf(buf,1024,logmsg,##__VA_ARGS__); \
    loginst->log(buf); \
  } while(0)


#define LOG_ERROR(logmsg,...) \
  do \
  { \
    std::unique_lock<std::mutex> locker(mm_mutex); \
    std::shared_ptr<Logger> loginst=Logger::instance(); \
    loginst->getLoglevel(ERROR); \
    char buf[1024]={0}; \
    snprintf(buf,1024,logmsg,##__VA_ARGS__); \
    loginst->log(buf); \
  } while(0)


  #define LOG_FATAL(logmsg,...) \
  do \
  { \
     std::unique_lock<std::mutex> locker(mm_mutex); \
    std::shared_ptr<Logger> loginst=Logger::instance(); \
    loginst->getLoglevel(FATAL); \
    char buf[1024]={0}; \
    snprintf(buf,1024,logmsg,##__VA_ARGS__); \
    loginst->log(buf); \
    exit(-1); \
  } while(0)

#ifdef  MUDBUG
#define LOG_DEBUG(logmsg,...) \
  do \
  { \
   std::unique_lock<std::mutex> locker(mm_mutex); \
    std::shared_ptr<Logger> loginst=Logger::instance(); \
    loginst->getLoglevel(DEBUG); \
    char buf[1024]={0}; \
    snprintf(buf,1024,logmsg,##__VA_ARGS__); \
    loginst->log(buf); \
  } while(0)
#else
    #define LOG_DEBUG(logmsg,...)
#endif

#endif


//启用时先设置同步/异步