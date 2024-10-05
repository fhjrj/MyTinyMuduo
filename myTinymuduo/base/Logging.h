#ifndef _LOGGING_
#define _LOGGING_
#include "Timestamp.h"
#include "LogStream.h"
#include <sys/time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <functional>


class Logger
{
    public:
    enum LogLevel
    {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        LEVEL_COUNT,//6个等级
    };

    class SourceFile//获得路径的文件名字，路径最后一个/
{
   public:
   explicit SourceFile(const char* filename):data_(filename)
   {
    const char* ll=strrchr(filename,'/');
    if(ll)
    {
        data_=ll+1;
    }
    size_=static_cast<int>(strlen(data_));
   }

   public:
   const char* data_;
   int size_;
};

 Logger(const char* file, int line);
 Logger(const char* file, int line, LogLevel level);
 Logger(const char* file, int line, LogLevel level, const char* func);
 ~Logger();

public:
  LogStream& stream(){
    return impl_.stream_;
 }//连续写入

 static LogLevel logLevel();//返回流的等级/日志等级
 static void setLogLevel (LogLevel);//设置流的等级/日志等级
 
 using OutputFunc=std::function<void(const char* meg,int len)>;
 using FlushFunc=std::function<void()>;
    //流输入输出相关的函数

 static void setOutput(OutputFunc);
 static void setFlush(FlushFunc);
 private:

 class Impl
 {
    public:
    using LogLevel=Logger::LogLevel;
    Impl(LogLevel level,int old_errno,const char* file,int line);
    void finish();
    void formatTime();
     Timestamp time_;
     LogStream stream_;//变化的
     LogLevel level_;//日志等级限制
     int line_;
    SourceFile basename_;//文件名字和长度
 };
 Impl impl_;
};



  extern Logger::LogLevel g_logLevel;//全局日志等级
  inline Logger::LogLevel Logger::logLevel()
  {
    return g_logLevel;
  }

const char* strerror_tl(int savedErrno);

#define LOG_DEBUG if (Logger::logLevel() <= Logger::DEBUG) \
  Logger(__FILE__, __LINE__, Logger::DEBUG, __func__).stream()
#define LOG_INFO if (Logger::logLevel() <= Logger::INFO) \
  Logger(__FILE__, __LINE__).stream()
#define LOG_WARN Logger(__FILE__, __LINE__, Logger::WARN).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::ERROR).stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::FATAL).stream()
/**
 * 当日志等级小于对应等级才会输出
 * 比如设置等级为FATAL，则logLevel等级大于DEBUG和INFO，DEBUG和INFO等级的日志就不会输出
 */
#endif


//一个Logger对应一个LogStream类对应一个Buffer类