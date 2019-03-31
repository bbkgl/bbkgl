#include "TcpConnection.h"
#include "Channel.h"
#include "EventLoop.h"
#include "base/Socket.h"
#include "Acceptor.h"

#include <cstdio>
#include <errno.h>
#include <iostream>

TcpConnection::TcpConnection(EventLoop *loop, const std::string &name, int sockfd, const InetAddress &local_addr,
                             const InetAddress &peer_addr) :
    loop_(loop),
    name_(name),
    state_(k_connecting),           // 构造TcpConnection对象就是为了建立连接，此时说明还没建立连接
    socket_(new Socket(sockfd)),
    channel_(new Channel(loop_, sockfd)),
    local_address_(local_addr),
    peer_address_(peer_addr)
{
    std::cout << "TcpConnection:ctor[" << name_ << "] at " << this
                                                           << " fd = " << sockfd << std::endl;
    channel_->SetReadCallback(std::bind(&TcpConnection::HandleRead, this));
}

TcpConnection::~TcpConnection()
{
    std::cout << "TcpConnection::dtor[" <<  name_ << "] at " << this
              << " fd = " << channel_->GetFd() << std::endl;
}

void TcpConnection::ConnEstablished()
{
    // 判断是不是在IO线程被调用
    loop_->AssertInLoopThread();
    // 判断是不是在连接状态
    assert(state_ == k_connecting);
    // 标记连接成功
    SetState(k_connected);
    // 在poller中注册读事件
    channel_->EnableReading();

    // conn_callback_本身由客户在Server中设置
    // 强制转换成智能指针类型
    conn_callback_(static_cast<std::shared_ptr<TcpConnection>>(this));
}

void TcpConnection::HandleRead()
{
    char buf[65536];
    // 读取客户端发送的消息
    ssize_t n = read(channel_->GetFd(), buf, sizeof(buf));
    // 通过回调函数与客户端进行通信
    msg_callback(static_cast<std::shared_ptr<TcpConnection>>(this), buf, n);
}