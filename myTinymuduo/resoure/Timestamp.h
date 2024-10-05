#ifndef _TIMEBASE_
#define _TIMEBASE_

#include <iostream>
#include <string>
#include <sys/types.h>
 class Timestamp{
 public:
  static const int kMicroSecondsPerSecond = 1000 * 1000;
  Timestamp();

  Timestamp(const Timestamp& p){
    this->mir=p.gettime();
  }
  explicit Timestamp(int64_t mir_);

  static Timestamp now();
  static Timestamp now_uper();
  std::string Tostring() const;
  
   time_t secondsSinceEpoch() const
  { return static_cast<time_t>(mir / kMicroSecondsPerSecond); }//在使用addTime构建的Timestamp上使用

  void invalude(){
    mir=0;
  }

  int64_t gettime() const {return this->mir;}//秒

  Timestamp addTime(Timestamp timestamp,double second)
  {
  int64_t delta=static_cast<int64_t>(second*kMicroSecondsPerSecond);
  return Timestamp(timestamp.gettime()+delta);
  }//微秒增加
  
  bool friend  operator <(Timestamp lhs_, Timestamp rhs_)
{
   return lhs_.gettime() < rhs_.gettime();
}

 bool friend operator==(Timestamp lhs, Timestamp rhs)
{
  return lhs.gettime() == rhs.gettime();
}

 private:
 int64_t mir;//微秒计数

 };

 #endif