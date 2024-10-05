#ifndef _LOGSTREAM_
#define _LOGSTREAM_

#include "noncopyable.h"
#include <string.h>
#include <sys/types.h>
#include <iostream>
#include <string>

class SP
{
    public:
    SP(const char* str,unsigned len):str_(str),len_(len)
    {

    }
    const char* str_;
    const unsigned len_; 
};


const int KSmallBuffer=4000;
const int KLargeBuffer=4000*1000;

 template<int SIZE>
 class FixedBuffer :public noncopyable
 {
    public:
    FixedBuffer():cur_(data_) //更新指针指向data
    {}
    ~FixedBuffer() {}
    public:
    char* current() {return cur_;}
    int avail() const{return static_cast<int>(end()-cur_);}//    返回剩余空间 
    void add(size_t len){cur_+=len;}//定位指针更新指针
    void append(const char* buf,size_t len)
    {    
        if(avail()>static_cast<int>(len))
        {
            memcpy(cur_,buf,len);
            cur_+=len;
        }
    }
    const char* data() const { return data_; }
    int length() const { return static_cast<int>(cur_ - data_); }//储存的内容长度

    void reset() {cur_=data_;}
    void bzero() {memset(data_,0,sizeof(data_));}
    private:
    const char* end() const{
        return data_+sizeof(data_);//即SIZE
    }
    char data_[SIZE];
    char* cur_=nullptr;//指标
 };//无扩容，满了自动丢弃LOG


 class  LogStream :public noncopyable//日志流肯定都是<<表示流流向 返回this*方便多次连续调用
 {
 public:
 using Buffer=FixedBuffer<KSmallBuffer>;
 LogStream& operator<<(bool v)
 {
buffer_.append(v?"1":"0",1);//buffer中存的是 01类字符串
return *this;
 }

    LogStream& operator<<(short);
    LogStream& operator<<(unsigned short);
    LogStream& operator<<(int);
    LogStream& operator<<(unsigned int);
    LogStream& operator<<(long);
    LogStream& operator<<(unsigned long);
    LogStream& operator<<(long long);
    LogStream& operator<<(unsigned long long);
    LogStream& operator<<(double);
    LogStream& operator<<(long double v);
    LogStream& operator<<(const void* );
    LogStream& operator<<(const std::string);
    LogStream& operator<<(const SP& p);
    LogStream& operator<<(float v)
          {
            *this<<static_cast<double>(v);
            return *this;
          }

    LogStream& operator<<(char v)
            {
                buffer_.append(&v,1);//增加1字符
                return *this;
            }

    LogStream& operator<<(const char* str)
     {
        if(str)
        {
            buffer_.append(str,strlen(str));
        }else{
            buffer_.append("null",6);//空字符 则追加空的
        }
    return *this;
     }

    LogStream& operator<<(const unsigned char* str) {
    return operator<<(reinterpret_cast<const char*>(str));
        }


    void append(const char* data,int len) {buffer_.append(data,len);}//len=sizeof(data)
    const Buffer& buffer() const {return buffer_;}
    void resetBuffer() {buffer_.reset();}
 private:
 template <typename T>
  void formatInteger(T);

 Buffer buffer_;
 static const int kMaxNumbersize=48;
 };

 #endif 