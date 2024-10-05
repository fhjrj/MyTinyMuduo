#include "Buffer.h"
#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>

ssize_t Buffer::readFd(int fd,int* saveErrno)//LT
{
     char extrabuf[65536];
     struct iovec vec[2];
     const size_t writebale=writableBytes();//剩余的，可写入的BUFFE字节大小
     vec[0].iov_base=begin()+writeIndex_;//总长度的WRITESIZE
     vec[0].iov_len=writebale;
     vec[1].iov_base=extrabuf;
     vec[1].iov_len=sizeof(extrabuf);
      //buffer中可写内存小于64k,则需要两个空间（包含extrabuf），若大于，只需要一个空间，就是Buffer,一次最多读64K
     const int iovcnt=(writebale<sizeof(extrabuf))?2:1;
     const ssize_t n=::readv(fd,vec,iovcnt);
     if(n<0){
        *saveErrno=errno;
     }
     else if(static_cast<size_t>(n)<=writebale){
        writeIndex_+=n;
     }else{//读取数据超过writable,buffer空间不够，要进行
        writeIndex_=buffer_.size();//扩容,Buffer直接变为现在长度
        append(extrabuf,n-writebale);
     }

     return n;//readv的好处就是提高读数据量的上限
}

ssize_t Buffer::writeFd(int fd, int *saveErrno)
{
    ssize_t n = ::write(fd, peek(), readableBytes());
    if (n < 0)
    {
        *saveErrno = errno;
    }
    return n;
}