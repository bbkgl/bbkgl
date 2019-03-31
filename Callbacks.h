//
// Created by bbkgl on 19-3-28.
//

#ifndef BBKGL_CALLBACKS_H
#define BBKGL_CALLBACKS_H

#include <functional>
#include <memory>

using TimerCallback = std::function<void()>;

class TcpConnection;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

// 连接回调
using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;

// 消息回调
using MessageCallback = std::function<void(const TcpConnectionPtr&, const char *, ssize_t)>;

#endif //BBKGL_CALLBACKS_H
