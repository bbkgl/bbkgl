//
// Created by bbkgl on 19-3-31.
//

#ifndef BBKGL_TCPSERVER_H
#define BBKGL_TCPSERVER_H

#include <boost/core/noncopyable.hpp>
#include <map>
#include <memory>
#include "Callbacks.h"
#include "TcpConnection.h"

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

class TcpServer : boost::noncopyable
{
public:
    TcpServer(EventLoop *loop, const InetAddress &listen_addr);
    ~TcpServer();

    void Start();

    // 设置线程数
    void SetThreadNum(int num);

    void SetConnCallback(const ConnectionCallback &cb) { conn_callback_ = cb; }

    void SetMsgCallback(const MessageCallback &cb) { msg_callback = cb; }

    void SetWriteCompleteCallback(const WriteCompleteCallback &cb)
    { write_complete_callback_ = cb; }

private:
    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;

    // 建立新连接
    void NewConnection(int sockfd, const InetAddress &peer_addr);

    // 删除一个连接
    void RemoveConnection(const TcpConnectionPtr &conn);

    void RemoveConnectionInLoop(const TcpConnectionPtr &conn);

    EventLoop *loop_;

    const std::string name_;

    // 服务器接收新连接的接收器
    std::unique_ptr<Acceptor> acceptor_;

    // 线程池
    std::unique_ptr<EventLoopThreadPool> thread_pool_;

    // 供用户设置的回调函数
    ConnectionCallback conn_callback_;
    MessageCallback msg_callback;
    WriteCompleteCallback write_complete_callback_;

    // 是否已经开始了运行
    bool started_;

    // 下一个连接的索引
    int next_conn_id_;

    // 根据名字设置的连接的映射
    ConnectionMap connections_;
};


#endif //BBKGL_TCPSERVER_H
