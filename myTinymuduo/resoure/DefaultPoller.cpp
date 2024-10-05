#include "Poller.h"
#include "EpollPoll.h"
#include <stdlib.h>
Poller* Poller::newDefaultPoller(Eventloop* loop){
    if(::getenv("MODUO_USE_POLL"))
    {
        return nullptr;//生成POLL实例
    }else{
        return new EPollPoller(loop);//初始化EPollpoller,然后初始化基类，但这个函数又在基类
    }
}
/*poller是基类，Epollpoller继承，这顺便就是父类指针储存子类*/
/*继承类一般构造基类，基类初始化构造继承类必须分开编写*/