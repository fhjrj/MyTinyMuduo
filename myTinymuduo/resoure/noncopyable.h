#ifndef _NONCOPY_
#define _NONCOPY_

class noncopyable{
  public:
  noncopyable(const noncopyable&)=delete;
  noncopyable& operator=(const noncopyable&)=delete;

  protected:
  noncopyable()=default;
   ~noncopyable()=default;
};

/*
被该类继承后，派生类对象无法进行拷贝构造和赋值，但是可以正常析构和构造
*/
#endif
