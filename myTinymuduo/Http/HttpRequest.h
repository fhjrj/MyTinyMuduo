#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include "Timestamp.h"
#include "noncopyable.h"
#include <map>
#include <string.h>
#include <sys/types.h>
#include <string>
//储存消息
class HttpRequest :public noncopyable
{
      public:
       enum class METHOD
       {
        GET=0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        PATH,
        Invalid
       };

       enum class VERSION
       {
         UNKNOW,
         HTTP10,
         HTTP11
       };

       HttpRequest():
         m_method_(METHOD::Invalid),
         m_version_(VERSION::UNKNOW),
         path_(),
         qurey_(),
         header_()
         {
         }

        void set_version(VERSION version_)
        {
            m_version_=version_;
        }
        
        VERSION Getversion() const
        {
            return m_version_;
        }        

        bool setMethod(const char* start,const char* end)//设置方法
        {
       std::string m(start,end);
       if(m==GET)
       {
        m_method_=METHOD::GET;
       }
       else if(m==POST)
       {
        m_method_=METHOD::POST;
       }
        else if(m==HEAD)
       {
        m_method_=METHOD::HEAD;
       }
        else if(m==PUT)
       {
        m_method_=METHOD::PUT;
       }
        else if(m==DELETE)
       {
        m_method_=METHOD::DELETE;
       }
        else if(m==TRACE)
       {
        m_method_=METHOD::TRACE;
       }
        else if(m==PATH)
       {
        m_method_=METHOD::PATH;
       }
       else
       {
        m_method_=METHOD::Invalid;
       }
       return m_method_==METHOD::Invalid?false:true;
        }


      const char* methodstring() const{//获得方法
        const char* result="UNKONWN";
         switch(m_method_)
     {
      case METHOD::GET :
        result = "GET";
        break;
      case METHOD::POST :
        result = "POST";
        break;
      case METHOD::HEAD :
        result = "HEAD";
        break;
      case METHOD::PUT :
        result = "PUT";
        break;
      case METHOD::DELETE :
        result = "DELETE";
        break;
       case METHOD::TRACE :
        result = "TRACE";
        break;
      case METHOD::PATH :
        result = "PATH";
        break;
      default:
        break;
    }
      return result;
      }

       
      const std::string& path() const
      { return path_; }
      
      void setPath(const char* start, const char* end)
      {
        const std::string path(start,end);
         path_=path;
      }
      
      void setQuery(const char* start, const char* end)
      {
        const std::string qurey(start,end);
        qurey_=qurey;
      }

     //获得请求头部对应的值
      std::string getHeader(const std::string& fiedld) const{
        std::string result;
        auto it=header_.find(fiedld);
        if(it!=header_.end())
        {
          result=it->second;
        }
        return result;
      }

      void addHeader(const char* start, const char* colon,const char* end)//HTTP消息头分析，start是一行开始，colon指向:，end是一行结尾，指向\r
      {
      std::string field(start,colon);//Content-length
      colon++;//跳过:
      colon+=strspn(colon," \t");//跳过空格
      std::string value(colon,end);
      while (!value.empty() && isspace(value[value.size()-1]))
    {
      value.resize(value.size()-1);
    }
        header_[field] = value;
      }

      const std::map<std::string, std::string>& headers() const
    {
        return header_;
    }

    void swap(HttpRequest& that)//安全操作
    {
      std::swap(m_method_,that.m_method_);
      std::swap(m_version_,that.m_version_);
      path_.swap(that.path_);
      qurey_.swap(that.qurey_);
      header_.swap(that.header_);
      std::swap(reveTime_,that.reveTime_);
    }
 
       const std::string& query() const 
       { return qurey_; }
     
      void setReceiveTime(Timestamp t)
      { reveTime_ = t; }

      Timestamp receiveTime() const
        { return reveTime_; }

      private:
      Timestamp reveTime_;
       std::string path_;
       std::string qurey_;
      std::map<std::string,std::string> header_;
      METHOD m_method_;
      VERSION m_version_;
      const std::string GET="GET";
      const std::string HEAD="HEAD";
      const std::string PUT="PUT";
      const std::string DELETE="DELETE";
      const std::string TRACE="TRACE";
      const std::string PATH="PATH";
      const std::string POST="POST";
};
#endif