#include "LogFile.h"
#include "FileUtil.h"
#include <assert.h>
#include <stdio.h>
#include <time.h>

 //basename一定要带有/
LogFile::LogFile(const std::string &basename, off_t rollSize, bool threadSafe, int flushInterval, int checkEveryN):
 basename_(basename),
    rollSize_(rollSize),
    flushInterval_(flushInterval),
    checkEveryN_(checkEveryN),
    count_(0),
    mutex_(),
    lastRoll_(0),
    lastFlush_(0),
    startOfperiod_(0L)
    {
         rollFile();
    }


 LogFile::~LogFile()=default;

 void LogFile::append(const char* logline,int len)
 {
    std::unique_lock<std::mutex> locker(mutex_);
    append_unlock(logline,len);
 }

void LogFile::flush()
{
    std::unique_lock<std::mutex> locker(mutex_);
    file_->flush();
}

void LogFile::append_unlock(const char* logline,int len){
    file_->append(logline,len);

    if(file_->writtenBytes()>rollSize_)
    {
        rollFile();//开创一个新的文件
    }else{
        count_++;//调用此函数的次数
        if(count_>=checkEveryN_)
        {
            count_=0;
            time_t now=::time(NULL);
            time_t thisPeriod=now/kpollpersecond*kpollpersecond;//计算现在是第几天 now/kRollPerSeconds求出现在是第几天，再乘以秒数相当于是当前天数0点对应的秒数
            if(thisPeriod!=startOfperiod_)
            {
                rollFile();//超过规定写入日志，进行重新创建
            }
            else if(now-lastFlush_>flushInterval_)//大于间隔 执行冲刷，间隔冲刷
            {
                lastFlush_=now;
                file_->flush();
            }
        }
    }
}

bool LogFile::rollFile()
{
    time_t now=0;
    std::string filename=getLogFileName(basename_,&now);//获得创建文件的时间
     time_t start = now / kpollpersecond * kpollpersecond;
     
      if (now > lastRoll_)
  { 
    count_=0;
    lastRoll_ = now;
    lastFlush_ = now;
    startOfperiod_ = start;
    file_.reset(new AppendFile(filename));//开启一个新的file实例储存
    return true;
  }
  return false;
}


//一个appendfile对应一个储存buffer
std::string LogFile::getLogFileName(const std::string basename, time_t *now)
{
    std::string filename;
    filename.reserve(basename.size()+64);
    filename=basename;

    char timebuf[32];
       struct tm tm;
    *now = time(NULL);
    localtime_r(now, &tm);
    strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S", &tm);//格式化时间
    filename += timebuf;

    filename += ".log";//追加格式化的时间+.log

    return filename;
}
