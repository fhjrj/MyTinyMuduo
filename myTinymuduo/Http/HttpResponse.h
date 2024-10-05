#ifndef HTTP_RESPOME_
#define HTTP_RESPOME_

#include <sys/types.h>
#include <string>
#include <map>

class Buffer;

 class HttpRespne
 {
    public:
    enum class HttpStatusCode
    {
      KUnknow,
      K200OK=200,
      K301MovedPermanently=301,
      K400BadRequest=400,
      K404NotFound=404,
    };

    explicit HttpRespne(bool close):
    statusCode_(HttpStatusCode::KUnknow),
    closeConnection_(close)
    {
    }

    void setStatusCode(HttpStatusCode code){
      statusCode_=code;
    }

    void setStatusMessage(const std::string& message)
    {
      statusMessgae_=message; 
    }

    void closeConnection(bool close)
    {
      closeConnection_=close;
    }

    bool getclose(){
      return closeConnection_;
    }

    void addHeader(const std::string& key,const std::string& value)
    {
      headers_[key]=value;
    }

     void setBody(const std::string& body)
    { body_ = body; }

     void setContentType(const std::string& contentType)
    { addHeader("Content-Type", contentType); }

    void appendToBuffer(Buffer* output) const;

    private:
    std::map<std::string,std::string> headers_;
    HttpStatusCode statusCode_;
    std::string statusMessgae_;
    bool closeConnection_;
    std::string body_;
 };



#endif