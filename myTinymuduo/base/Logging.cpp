#include "CurrentThread.h"
#include "Timestamp.h"
#include "Logging.h"
#include <errno.h>
#include <stdio.h>
#include <sstream>
#include <string.h>



namespace ThreadInfo
{
__thread char t_errnobuf[512];
__thread char t_time[64];
__thread time_t t_lastSecond;

};

const char* strerror_tl(int savedErrno)
{
    return  strerror_r(savedErrno, ThreadInfo::t_errnobuf, sizeof (ThreadInfo::t_errnobuf));
}//错误储存在此中

Logger::LogLevel initLogLevel()
{
    if(::getenv("MUDUO_LOG_TRACE"))
      return Logger::TRACE;
      else if(::getenv("MUDUO_LOG_DEBUG"))
    return Logger::DEBUG;
    else
    return Logger::INFO;
}

Logger::LogLevel g_logLevel=initLogLevel();//初始化日志等级

const char* LogLevelName[Logger::LEVEL_COUNT]//// 根据Level返回Level名字
{
    "TRACE ",
    "DEBUG ",
    "INFO  ",
    "WARN  ",
    "ERROR ",
    "FATAL ",
};
//存入的是char*类数组，每个元素都是一个字符串
void defaultOutput(const char* msg,int len)
{
    size_t n=fwrite(msg,1,len,stdout);//将信息标准输出
    (void)n;
}

void defaultFlush()
{
    fflush(stdout);
}

Logger::OutputFunc g_output=defaultOutput;
Logger::FlushFunc g_flush=defaultFlush;
//指定函数
Logger::Impl::Impl(LogLevel level,int savedErrno,const char*  file,int line)
:time_(Timestamp::now_uper()),//现在的时间微秒
 stream_(),//默认构造，没啥可构造的
 level_(level),
 line_(line),
 basename_(file)//文件名字
 {
    formatTime();
    CurrentThread::tid();
    char c=' ';
    stream_<<c;
    stream_<<SP(LogLevelName[level],6);//载入日志模式
     if (savedErrno != 0)
  {
    stream_ << strerror_tl(savedErrno) << " (errno=" << savedErrno << ") ";
  }  
 }//stream_中现在储存的是时间+日志等级

 void Logger::Impl::formatTime()//存入时间
 { 
    int64_t mirtime=time_.gettime();
    int miroseconds=static_cast<int>(mirtime%Timestamp::kMicroSecondsPerSecond);//计算微秒
    time_t second=time(NULL);
    struct tm* this_tm=localtime(&second);
    struct tm now=*this_tm;
    snprintf(ThreadInfo::t_time,sizeof(ThreadInfo::t_time),"%4d_%02d_%02d %02d:%02d:%02d",
    now.tm_year+1900,now.tm_mon+1,now.tm_mday,now.tm_hour,now.tm_min,now.tm_sec);
    ThreadInfo::t_lastSecond=second;

    char buf[32]={0};
    snprintf(buf,sizeof(buf),"%06d",miroseconds);//存入buf中,是6位
    stream_<<SP(ThreadInfo::t_time,17)<<SP(buf,7);//7是增加一个空格
//流中存入的是时间
 }

// level默认为INFO等级
Logger::Logger(const char* file, int line)  
    : impl_(INFO, 0, file, line)
{
}

Logger::Logger(const char* file, int line, Logger::LogLevel level)
    : impl_(level, 0, file, line)
{
}

// 可以打印调用函数
Logger::Logger(const char* file, int line, Logger::LogLevel level, const char* func)
  : impl_(level, 0, file, line)
{
    impl_.stream_ << func << ' ';
}


void Logger::Impl::finish()
{
    stream_<<"-"<<SP(basename_.data_,basename_.size_)<<":"<<line_<<'\n';
}
 Logger::~Logger()
 {
    impl_.finish();//文件名字内容和文件名字进行追加
    const LogStream::Buffer& buf=stream().buffer();//Impl_-->LogStream--->fixbuffer,将fixedbuffer取出来，注意是引用
    g_output(buf.data(),buf.length());//输出
    if(impl_.level_==FATAL)//是FATAL 直接终止程序
    {
        g_flush();
        abort();

    }
 }

 void Logger::setLogLevel(Logger::LogLevel level)
 {
    g_logLevel=level;
 }

//设置输入输出函数
 void Logger::setOutput(OutputFunc out)
 {
    g_output=out;
 }

void Logger::setFlush(FlushFunc flush)
{
  g_flush = flush;
}

