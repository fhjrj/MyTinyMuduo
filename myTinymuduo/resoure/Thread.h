#ifndef _THREAD_
#define _THREAD_

#include "noncopyable.h"
#include <functional>
#include <thread>
#include <memory>
#include <unistd.h>
#include <string>
#include <sys/types.h>
#include <atomic>

class Thread:public noncopyable
{
public:
using ThreadFunc=std::function<void()>;
explicit Thread(ThreadFunc cb,const std::string&name);
Thread():
started_(false),
joined_(false),
tid_(0),
func_(ThreadFunc()),
name_(std::string())
{
    setdefaltname();
}
~Thread();

void start();
void join();

bool started() const {return started_;}
pid_t tid() const {return tid_;}

const std::string& name() {
    return name_;
}

static int numCreate() {return numthread_.load();}
 void setdefaltname() const;
private:
 bool started_;
 bool joined_;
 std::shared_ptr<std::thread> thread_;//只能指针 可以控制线程开始
 pid_t tid_;
 ThreadFunc func_;
 mutable std::string name_;
 static std::atomic<int> numthread_;
};


#endif