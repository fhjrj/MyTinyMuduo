#ifndef _SOCKET_
#define _SOCKET_

#include "noncopyable.h"
#include "Inetaddress.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>



class Socket :public noncopyable
{
   public:
   explicit Socket(int sockfd):sockfd_(sockfd)
    {}
   
   ~Socket();
   int fd() const{
    return sockfd_;
   }

   void bindAddress(const InetAddress& address);
   void listen();
   int accept(InetAddress* address);
   void shutdownwrite();
   void setTcpNoDelay(bool a);
   void setReuseAddr(bool a);
   void setReusePoot(bool a);
   void setKeepalive(bool a);
  private:
  const int sockfd_;
};
#endif 