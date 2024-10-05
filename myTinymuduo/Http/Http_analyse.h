#ifndef HTTP_ANALYSE
#define HTTP_ANALYSE

#include "HttpRequest.h"
class Buffer;

   class HttpAnalyse
   {
    public:
    enum class Check_State
    {
        CHECK_STATE_REQUESTLINE = 0,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT,
        CHECK_STATE_OVER
    };
    
    bool parseRequest(Buffer* buffer,Timestamp recvetime);

    HttpAnalyse():m_state_(Check_State::CHECK_STATE_REQUESTLINE)
    {
    }

    bool isover(){
        return m_state_==Check_State::CHECK_STATE_OVER;
    }

    HttpRequest& getrequset() {
        return request_;
    }
    void reset()
    {
       m_state_=Check_State::CHECK_STATE_REQUESTLINE;
       HttpRequest dummy;
       request_.swap(dummy);
    }
    private:
    bool processHeadRequest(const char* begin,const char* end);
    HttpRequest request_;
    Check_State m_state_;
   };
#endif