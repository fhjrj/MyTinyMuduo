#ifndef BASE_FILE_H
#define BASE_FILE_H

#include "noncopyable.h"
#include <stdio.h>
#include <string>

  class AppendFile :public noncopyable
  {  
     public:
     AppendFile(std::string);
     void flush();
     ~AppendFile();
     void append(const char* ,const size_t);
     off_t writtenBytes() const { return writenBytes; }
      size_t write(const char*,const size_t);
     private:
      FILE* m_fp;
      char buffer_[54*1024];
      off_t writenBytes; 
  };

#endif