#include "Aslogging.h"
#include "Logging.h"
#include "Timestamp.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>

static const off_t kRollSize = 1*1024*1024;
AsyncLogging* g_asyncLog = NULL;

inline AsyncLogging* getAsyncLog()
{
    return g_asyncLog;
}

void test_Logging()
{
    LOG_DEBUG << "debug";
    LOG_INFO << "info";
    LOG_WARN << "warn";
    LOG_ERROR << "error";

    const int n = 10;
    for (int i = 0; i < n; ++i) {
        LOG_INFO << "Hello, " << i << " abc...xyz";
    }
}

void test_AsyncLogging()
{
    const int n = 100;
    for (int i = 0; i < n; ++i) {
        LOG_INFO << "Helloooo, " << i << " abc...xyz";
    }
}

void asyncLog(const char* msg, int len)
{
    AsyncLogging* logging = getAsyncLog();
    
    if (logging)
    {    
        
        logging->append(msg, len);
    }
}

int main(int argc, char* argv[])
{
    printf("pid = %d\n", getpid());

    AsyncLogging log(::basename(argv[0]), kRollSize);
    test_Logging();

    sleep(1);

    g_asyncLog = &log;
    Logger::setOutput(asyncLog); 
    log.start();
     // 指定LOGSTREAM的函数为异步日志

    test_Logging();
    test_AsyncLogging();

    sleep(1);
    log.stop();
    return 0;
}

/*

    LOG_INFO--->返回stream对象的引用，以用于连续使用<<,储存在LogStream::FixBuffer<Ksmallsize>中
    Logger::Logger------>初始化impl

    流的储存输出和异步日志的储存输出不用

    Logfile---->根据传入的路径创建日志----->Logfile调用FileUtil方法进行写。
    LogFile控制滚动日志的创建和冲刷时间，
    
*/