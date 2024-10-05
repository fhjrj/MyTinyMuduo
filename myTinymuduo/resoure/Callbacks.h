#ifndef _CALLBACKS_
#define _CALLBACKS_

#include <functional>
#include <memory>
#include "Buffer.h"
#include "Timestamp.h"

class Buffer;
class TcpConnection;

using TcpConnectionPtr=std::shared_ptr<TcpConnection>;
using Timercallback=std::function<void()>;
using ConnectionCallback=std::function<void(const TcpConnectionPtr&)>;
using CloseCallback=std::function<void(const TcpConnectionPtr&)>;
using WriteCompleteCallback=std::function<void(const TcpConnectionPtr&)>;
using MessageCallback=std::function<void(const TcpConnectionPtr&,Buffer*,Timestamp)>;
using HighWaterMarkCallback=std::function<void(const TcpConnectionPtr&,size_t)>;//类似于TCP中的流量控制
#endif