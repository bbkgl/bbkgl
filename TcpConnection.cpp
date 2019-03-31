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
    std::cout << "(Close)TcpConnection::dtor[" <<  name_ << "] at " << this
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
    // 哇，这里贼坑。陈硕书上继承的是boost::enable_shared_from_this<TcpConnection>
    // 可是我的conn_callback_()中的参数却是std::shared_ptr<TcpConnection>，所以一直报错
    // 后面为了让CLion不报错，只能用static_cast强制转换，结果出现了conn_callback_回调结束后，
    // TcpConnection对象直接析构的错误。。。。。
    // 后面找到了一篇文章：https://blog.csdn.net/adream307/article/details/85116845
    // 发现可以获得this指针的shared_ptr。。。。
    // 然后将继承对象改成std::enable_shared_from_this<TcpConnection>，
    // 可以使用shared_from_this()直接获得this指针的shared_ptr。
    conn_callback_(shared_from_this());
}

void TcpConnection::HandleRead()
{
    char buf[65536];
    // 读取客户端发送的消息
    ssize_t n = read(channel_->GetFd(), buf, sizeof(buf));
    // 通过回调函数与客户端进行通信
    // msg_callback_本身由客户在Server中设置
    // 哇，这里贼坑。陈硕书上继承的是boost::enable_shared_from_this<TcpConnection>
    // 可是我的msg_callback_()中的参数却是std::shared_ptr<TcpConnection>，所以一直报错
    // 后面为了让CLion不报错，只能用static_cast强制转换，结果出现了conn_callback_回调结束后，
    // TcpConnection对象直接析构的错误。。。。。
    // 后面找到了一篇文章：https://blog.csdn.net/adream307/article/details/85116845
    // 发现可以获得this指针的shared_ptr。。。。
    // 然后将继承对象改成std::enable_shared_from_this<TcpConnection>，
    // 可以使用shared_from_this()直接获得this指针的shared_ptr。
    msg_callback(shared_from_this(), buf, n);
}