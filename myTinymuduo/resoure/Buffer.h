#ifndef _BUFFER_
#define _BUFFER_

#include <assert.h>
#include <string.h>
#include <vector>
#include <string>
#include <algorithm>
#include <sys/types.h>

//readindex<=writeindex<<allsize,读写公用一个buffer
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |     已读区域       |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size

class Buffer
{
   public:
   static const size_t kCheapPrepend=8;
   static const size_t kInitialSize=1024;
   ~Buffer()=default;
   explicit Buffer(size_t initialSize=kInitialSize):buffer_(kCheapPrepend+kInitialSize),
   readerIndex_(kCheapPrepend),writeIndex_(kCheapPrepend)
   {}
   //先进行BUFFER的初始化
   
   void swap(Buffer& rhs){
    buffer_.swap(rhs.buffer_);
    std::swap(readerIndex_,rhs.readerIndex_);
    std::swap(writeIndex_,rhs.writeIndex_);
   }//进行数据的交换

   size_t readableBytes() const{
    return writeIndex_-readerIndex_;//还有多少数据未读
   }

   size_t writableBytes() const{
    return buffer_.size()-writeIndex_;
   }

   size_t prependableBytes() const{
    return readerIndex_;//返回已经读了多少字节了
   }
 //返回可读的内容起始定位
   const char* peek() const{
     return begin()+readerIndex_;
   }
   
   /*预读函数----> to prevent*/
   void retrieve(size_t len){
     assert(len<=readableBytes());//预读长度是否小于可读的长度
     if(len<readableBytes()){
        readerIndex_+=len;//
     }else{
        retrieveall();
        }
   }
   /*对于客户端 先将对面传入的数据 写入Buffer中，writesize=写入数据，这时readsize=8*/
   /*回写入对面信息时。使用readsize*/
  
  //进行扩容准备，先前已经准备了
void retrieveUntil(const char* end)
  {
    assert(peek() <= end);
    assert(end <= beginWrite());
    retrieve(end - peek());
  }

   void retrieveall(){
    readerIndex_=kCheapPrepend;
    writeIndex_=kCheapPrepend;
   }//重置
   
   //将buffer中还未读的读完 readerIndex<writeIndex ----->readerInder=writeIndex
   std::string retrieveAllAsString(){
    return retrieveAsString(readableBytes());
   }
   
   /*往后(readerindex_)开始，取还未读取的内容*/
   std::string  retrieveAsString(size_t len){
     assert(len<=readableBytes());
     std::string result(peek(),len);
     retrieve(len);//更新readerIndex_
     return result;
   }


   void append(const char* data,size_t len){
    ensureWritableBytes(len);
    std::copy(data,data+len,beginWrite());//追加
    hasWritten(len);//更新
   }

   void append(const void* data,size_t len){
    append(static_cast<const char*>(data),len);
   }

   void ensureWritableBytes(size_t len){
    if(writableBytes()<len){//buffer中还可以写入的数字小于欲写的字节，进行扩容
        makeSpace(len);
    }else{
        assert(writableBytes()>=len);
    }
   }
  
  char* beginWrite(){
    return begin()+writeIndex_;
  }

  const char* beginWrite() const {
    return begin()+writeIndex_;
  }

  void hasWritten(size_t len){
   // assert(len<=readableBytes());
    writeIndex_+=len;
  }
  
  const char* findCRLF() const
    {
        const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF+2);
        return crlf == beginWrite() ? NULL : crlf;
    }


  void unwrite(size_t len){
    assert(len<=readableBytes());
    writeIndex_-=len;
  }
 
  void prepend(const void* data,size_t len){
     assert(len <= prependableBytes());
    readerIndex_ -= len;
    const char* d = static_cast<const char*>(data);
    std::copy(d, d+len, begin()+readerIndex_);
  }
  /*传入的字节数len<已读的字节数，从已读最后面，新的数据在后面进行覆盖*/

  size_t interalCapacity() const{
    return buffer_.capacity();
  }

  ssize_t readFd(int fd,int* saveErrno);
   ssize_t writeFd(int fd, int *saveErrno); 

   std::string to_string(){
    if(buffer_.size()==0) return std::string{};
    std::string p(buffer_.begin(),buffer_.end());
    return p;
   }

   const char kCRLF[3]="\r\n";
   private:
  char* begin(){
    return &*buffer_.begin();
  }

  const char* begin() const{
    return &*buffer_.begin();
  }

   void makeSpace(size_t len){
      if(writableBytes()+prependableBytes()<len+kCheapPrepend){
        buffer_.reserve(writeIndex_+len);//扩容更多内容
      }else{
     /*未超过BUFFER*/
       assert(kCheapPrepend<readerIndex_);
       size_t readable=readableBytes();//算出还有多少数据还未读
       std::copy(begin()+readerIndex_,begin()+writeIndex_,
       begin()+kCheapPrepend);
       readerIndex_=kCheapPrepend;//重置
       writeIndex_=readerIndex_+readable;//重置
       assert(readable == readableBytes());
   }
   }
   /* Buffer中还没有读 这时readsize=8,剩下的还可以写的字节数量<需要写入的字节数，这时就要进行扩容*/
  /*先进行读再进行写，不是读写并行*/
  /*但一个Buffer可能进行多次读写*/
   std::vector<char> buffer_;
   size_t readerIndex_;
   size_t writeIndex_;
  
};

   

#endif