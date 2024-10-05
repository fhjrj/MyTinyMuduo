#include "FileUtil.h"
#include <string>
#include "Logging.h"
#include <unistd.h>


 AppendFile::AppendFile(std::string name):m_fp(::fopen(name.c_str(),"a")),
  writenBytes(0)
  {
    // 将m_fp缓冲区设置为本地的buffer_，文件i/o是先要写进缓冲再拷贝进磁盘 所以将buffer_作为文件指针的缓冲区
    //然后拷贝到磁盘后，会在文本显示
    ::setbuffer(m_fp, buffer_, sizeof(buffer_));
  }
   
     AppendFile::~AppendFile()
     {
        ::fclose(m_fp);
     }

     void  AppendFile::append(const char* logline, const size_t len)//要写多少数据
     {
        size_t written=0;
        while(written!=len)
        {
            size_t remain=len-written;
            size_t n=write(logline+written,remain);//logline写，logline顺便同步更新指针
            if(n!=remain)
            {
                int err=ferror(m_fp);
                 if (err)
      {
        fprintf(stderr, "AppendFile::append() failed %s\n", strerror_tl(err));
        break;
      }
    }
             written += n;//更新写入数据的大小
            }
           writenBytes+=written;
        }


        void AppendFile::flush()
        {
            ::fflush(m_fp);
        }

          size_t AppendFile:: write(const char* data,const size_t len)
          {
            return ::fwrite_unlocked(data,1,len,m_fp);
            /*
            参数一 指向待写入数据块的指针，参数二 每个数据的大小 sizeof(char)=1 参数三是数据类型
            参数四是文件之指针            */
          }




      
     
    