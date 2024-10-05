#ifndef LOG_FILE
#define LOG_FILE

#include <memory>
#include <mutex>
#include <string>
#include <sys/types.h>
#include "FileUtil.h"
#include "noncopyable.h"
class AppendFile;
 class LogFile : public noncopyable
 {   
    public:
     LogFile(const std::string& basename=std::string(),
          off_t rollSize=0L,
          bool threadSafe=true,
          int flushInterval = 3,
          int checkEveryN = 1024);
  ~LogFile();
   void append(const char* logline, int len);
   void flush();
    bool rollFile();

     private:
     void append_unlock(const char* logline,int len);
     static std::string getLogFileName(const std::string basename,time_t* now);

     const std::string basename_;
     const off_t rollSize_;//滚动日志
     const int flushInterval_;//fflush间隔
     const int checkEveryN_;

     int count_;
     std::mutex mutex_;
     time_t startOfperiod_;
     time_t lastRoll_;
     time_t lastFlush_;
     std::unique_ptr<AppendFile> file_;
     const static int kpollpersecond=60*60*24;
 };
#endif