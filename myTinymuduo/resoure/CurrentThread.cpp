#include "CurrentThread.h"
#include <sys/types.h>

namespace CurrentThread
{
    __thread int t_cachedTid=0;

    void cacheTid()
    {
        if(t_cachedTid==0){
t_cachedTid=static_cast<pid_t>(::syscall(SYS_gettid));
/*通过系统调用，获得当前线程的id*/
        }
    }

}
