#ifndef _INETADDRESS_
#define _INETADDRESS_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <sys/types.h>

 class InetAddress//管理sockaddr_In
 {
    public:
    explicit InetAddress(uint16_t port_=0,std::string Ip_="127.0.0.1");
    explicit InetAddress(const sockaddr_in &addr):address(addr){}

     std::string get_ip()const;
     std::string get_ip_port() const;
     uint16_t get_port() const;
      const sockaddr_in* getsockaddr() const{
      return &address;
     }
     void setSockAddr(const sockaddr_in& addr){
       address=addr;
     }
    private:
    sockaddr_in address;
 };

 #endif