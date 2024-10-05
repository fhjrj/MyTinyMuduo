#include "Inetaddress.h"
#include <string.h>

InetAddress::InetAddress(uint16_t port_,std::string ip_){
 ::memset(&address,0,sizeof(address));
 address.sin_family=AF_INET;
 address.sin_port=::htons(port_);
 address.sin_addr.s_addr=::inet_addr(ip_.c_str());
}

std::string InetAddress::get_ip() const{
    char buf[64]={0};
    ::inet_ntop(AF_INET,&address.sin_addr,buf,sizeof(buf));
    return buf;
}

std::string InetAddress::get_ip_port() const{
    char buf[64]={0};
    ::inet_ntop(AF_INET,&address.sin_addr,buf,sizeof(buf));//IP地址将网络字节序转化为点分十进制字符串
    size_t end=strlen(buf);
    uint16_t port=ntohs(address.sin_port);
    sprintf(buf+end,":%u",port);
    return buf;
}

uint16_t InetAddress::get_port() const {
    return ntohs(address.sin_port);
}
