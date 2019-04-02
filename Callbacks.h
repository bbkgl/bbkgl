//
// Created by bbkgl on 19-3-28.
//

#ifndef BBKGL_CALLBACKS_H
#define BBKGL_CALLBACKS_H

#include <functional>
#include <memory>
#include "Timestamp.h"

class TcpConnection;
class Buffer;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

using TimerCallback = std::function<void()>;

// 连接回调
using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;

// 消息回调
using MessageCallback = std::function<void(const TcpConnectionPtr&, Buffer *, Timestamp)>;

// 客户端连接主动挂起回调
using CloseCallback = std::function<void(const TcpConnectionPtr &)>;

#endif //BBKGL_CALLBACKS_H
