#ifndef _CurrentThread_
#define _CurrentThread_

#include <cxxabi.h>
#include <execinfo.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
namespace CurrentThread
{
     extern __thread int t_cachedTid;

     void cacheTid();

     inline int tid()
     {
        if(__builtin_expect(t_cachedTid==0,0))
        {
            cacheTid();
        }//未获得当前线程ID,则取获取
        return t_cachedTid;
     }
}

#endif