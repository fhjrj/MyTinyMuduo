#include "LogStream.h"
#include <string.h>
#include <algorithm>
#include <limits>

const char digits[] = "9876543210123456789";
const char* zero = digits + 9;//----->*zero='0'


template class FixedBuffer<KSmallBuffer>;
template class FixedBuffer<KLargeBuffer>;

template<typename T>
size_t convert(char buf[],T value)
{
    T i=value;
    char* p=buf;

    do
    {
         int lsd=static_cast<int>(i%10);//9434
         i/=10;
        *p++=zero[lsd];//--->翻转存入
    }while(i!=0);

    if(value<0)
    {
        *p++='-';
    }
    *p='\0';
    std::reverse(buf,p);//---->revser ---> 9434 ok
    return p-buf;//数字长度
}

// buffer容不下kMaxNumericSize个字符的话会被直接丢弃
template<typename T>
void LogStream::formatInteger(T v)
{
    if(buffer_.avail()>=kMaxNumbersize)
    {
        size_t len=convert(buffer_.current(),v);//现在位置进行追加
        buffer_.add(len);//更新指针
    }
}



LogStream& LogStream::operator<<(const SP& v)
{
  buffer_.append(v.str_,v.len_);
  return *this;
}


//数字类型变为字符串储存
 LogStream& LogStream::operator<<(short v)
 {   
    *this<<static_cast<int>(v);
     return *this;
 }
    LogStream& LogStream::operator<<(unsigned short v){
        *this<<static_cast<unsigned int>(v);
        return *this;
    }
    LogStream& LogStream::operator<<(int v)
    {
        formatInteger(v);
        return *this;
    }
    LogStream& LogStream::operator<<(unsigned int v){
         formatInteger(v);
          return *this;
    }
    LogStream&  LogStream::operator<<(long v){
          formatInteger(v);
          return *this;
    }
    LogStream&  LogStream::operator<<(unsigned long v){
          formatInteger(v);
          return *this;
    }
    LogStream& LogStream::operator<<(long long v){
        formatInteger(v);
      return *this;
    }
    LogStream&  LogStream::operator<<(unsigned long long v){
         formatInteger(v);
         return *this;
    }

  LogStream& LogStream::operator<<(double v) {
  if (buffer_.avail() >= kMaxNumbersize) {
    int len = snprintf(buffer_.current(), kMaxNumbersize, "%.12g", v);//高精度追加
    buffer_.add(len);
  }
  return *this;
}
//整型及其以上的追加

LogStream& LogStream::operator<<(long double v) {
  if (buffer_.avail() >= kMaxNumbersize) {
    int len = snprintf(buffer_.current(),kMaxNumbersize, "%.12Lg", v);
    buffer_.add(len);
  }
  return *this;
}

 LogStream& LogStream::operator<<(const std::string p)
 {
    size_t size=p.size();
    buffer_.append(p.c_str(),size);
    return *this;
 }


 LogStream& LogStream::operator<<(const void* data) 
{
    *this << static_cast<const char*>(data); 
    return *this;
}
//消息流 


