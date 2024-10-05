
#include "Timestamp.h"
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
//#include <iostream>
Timestamp::Timestamp():mir(0){}

Timestamp::Timestamp(int64_t mir_):mir(mir_) {}

Timestamp Timestamp::now(){
  time_t timee=time(NULL);
  
  return Timestamp(timee);
}

Timestamp Timestamp::now_uper()
{
 
  struct timeval tv;
  gettimeofday(&tv, NULL);
  int64_t seconds = tv.tv_sec;
  return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}


std::string Timestamp::Tostring() const
{
    char buf[128]={0};
    tm* m_time=localtime(&mir);
    snprintf(buf,128,"%4d_%02d_%02d %02d:%02d:%02d",m_time->tm_year+1900,m_time->tm_mon+1
    ,m_time->tm_mday,m_time->tm_hour,m_time->tm_min,m_time->tm_sec);
    return buf;
}
/*
int main(){
  Timestamp p=Timestamp::now();
  std::cout<<p.gettime()<<std::endl;
  auto g=p.addTime(p,3.0);
   std::cout<<p.gettime()<<std::endl;
      std::cout<<g.gettime()<<std::endl;
  
}
*/