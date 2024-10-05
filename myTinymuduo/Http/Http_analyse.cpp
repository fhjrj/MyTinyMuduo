#include "Http_analyse.h"
#include "Buffer.h"
#include <map>
#include <algorithm>
#include <iostream>
#include "HttpRequest.h"

bool HttpAnalyse::processHeadRequest(const char* begin,const char* end)//end--->指向\r
{
    bool succeed=false;
    const char* start=begin;
    const char* space=std::find(start,end,' ');//找到第一个空格的位置__method
    if(space!=end&&request_.setMethod(start,space))
    {
        space++;
        start=std::find(space,end,' ');//找到第二个空格 start
        if(start!=end)
        {
            const char* question=std::find(space,start,'?');//说明带有参数
            if(question!=start)
            {     
                  request_.setPath(space,question);
                  request_.setQuery(question,start);//ugl
            }else{
                request_.setPath(space,question);
            }
        }
        space=start;
        space++;
        succeed=(end-space==8&&std::equal(space,end-1,"HTTP/1."));//end在下标为8的地方 第9位字符,
        if(succeed)//[start,end-1)
        {
            if(*(end-1)=='1')
            {
                request_.set_version(HttpRequest::VERSION::HTTP11);
            }
            else if(*(end-1)=='0')
            {
                request_.set_version(HttpRequest::VERSION::HTTP10);
            }else{
                succeed=false;
            }
        }
    }
return succeed;
}


bool HttpAnalyse::parseRequest(Buffer* buffer,Timestamp recvetime)//buffer中储存着http接受报文
{
    bool ok=false;
    bool hasmore=true;
    while(hasmore)
    {
        if(m_state_==Check_State::CHECK_STATE_REQUESTLINE)
        {
            const char* crlf=buffer->findCRLF();//\r
            if(crlf)
            {    
                ok=processHeadRequest(buffer->peek(),crlf);
                if(ok)
                {
                    request_.setReceiveTime(recvetime);                  
                    buffer->retrieveUntil(crlf+2);//更新位置 
                    m_state_=Check_State::CHECK_STATE_HEADER;
                }else{
                    hasmore=false;
                }
            }else{
                 hasmore=false;
            }
        }
        else if(m_state_==Check_State::CHECK_STATE_HEADER)
        {
            const char* pr=buffer->findCRLF();//将消息头的每一行进行分割出来，
            if(pr)
            {
                const char* colon=std::find(buffer->peek(),pr,':');//
                if(colon!=pr)
                {
            request_.addHeader(buffer->peek(),colon,pr);//添加状态首部
                }
                else//无":"
                {   
                    const char* p=colon+2;// *colon='\r'
                    if(*p=='\0')
                    {
                    m_state_ = Check_State::CHECK_STATE_OVER;
                    hasmore = false;
                    }
                    else if(*p!='\0')
                    {
                        m_state_=Check_State::CHECK_STATE_CONTENT;
                    }
                }
                buffer->retrieveUntil(pr+2);
            }else{
                 hasmore = false;
            }
        }
        else if(m_state_==Check_State::CHECK_STATE_CONTENT)//消息体不做任何处理
        {
         /*
         doing something
         */
        m_state_=Check_State::CHECK_STATE_OVER;
        hasmore=false;
        }
    }
                return ok;
}
