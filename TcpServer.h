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

class TcpServer : boost::noncopyable
{
public:
    TcpServer(EventLoop *loop, const InetAddress &listen_addr);
    ~TcpServer();

    void Start();

    void SetConnCallback(const ConnectionCallback &cb) { conn_callback_ = cb; }

    void SetMsgCallback(const MessageCallback &cb) { msg_callback = cb; }

private:
    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;

    // 建立新连接
    void NewConnection(int sockfd, const InetAddress &peer_addr);

    EventLoop *loop_;

    const std::string name_;

    // 服务器接收新连接的接收器
    std::unique_ptr<Acceptor> acceptor_;

    // 供用户设置的回调函数
    ConnectionCallback conn_callback_;
    MessageCallback msg_callback;

    // 是否已经开始了运行
    bool started_;

    // ？？？
    int next_conn_id_;

    // 根据名字设置的连接的映射
    ConnectionMap connections_;
};


#endif //BBKGL_TCPSERVER_H
