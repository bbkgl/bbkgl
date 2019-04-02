#include "TcpConnection.h"
#include "Channel.h"
#include "EventLoop.h"
#include "base/Socket.h"
#include "Acceptor.h"
#include "base/SocketsOpts.h"

#include <cstdio>
#include <errno.h>
#include <iostream>
#include <cstring>

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
    channel_->SetReadCallback(std::bind(&TcpConnection::HandleRead, this, std::placeholders::_1));
    // 其实我觉得下面三句都是没必要的，因为HandleRead()本来就会调用下面的三个函数，
    // 最后也是在Channel::read_callback()中被调用，实际上Channel::read_callback()
    // 就会调用TcpConnection::HandleRead()，HandleRead()根据情况调用以下三个函数，
    // 然后那传进去的目的又是什么呢？到时候把下面三行注释掉
    channel_->SetCloseCallback(std::bind(&TcpConnection::HandleClose, this));
    channel_->SetWriteCallback(std::bind(&TcpConnection::HandleWrite, this));
    channel_->SetErrorCallback(std::bind(&TcpConnection::HandleError, this));
}

TcpConnection::~TcpConnection()
{
    std::cout << "(Close)TcpConnection::dtor[" <<  name_ << "] at " << this
              << " fd = " << channel_->GetFd() << std::endl;
}

void TcpConnection::Send(const std::string &message)
{
    if (state_ == k_connected)
    {
        // 保证线程安全
        if (loop_->IsInLoopThread())
            SendInLoop(message);
        else
            loop_->RunInLoop(std::bind(&TcpConnection::SendInLoop, this, message));
    }
}

void TcpConnection::SendInLoop(const std::string &message)
{
    loop_->AssertInLoopThread();
    ssize_t nwrote = 0;
    // 如果没有数据在output_buffer_中，则试图往系统的发送缓冲区直接写入
    if (!channel_->IsWriting() && output_buffer_.ReadableBytes() == 0)
    {
        nwrote = write(channel_->GetFd(), message.data(), message.size());
        // 如果数据发送成功了
        if (nwrote >= 0)
        {
            if (static_cast<size_t>(nwrote) < message.size())
                std::cout << "I am going to write more data\n";
        }
        else
        {
            nwrote = 0;
            if (errno == EWOULDBLOCK)
                std::cerr << "TcpConnection::SendInLoop()\n";
        }
    }
    // 断言
    assert(nwrote >= 0);
    // 如果一次性没有全部发出去
    if (static_cast<size_t>(nwrote) < message.size())
    {
        // 存到input_buffer_中
        output_buffer_.Append(message.data() + nwrote, message.size() - nwrote);
        // 继续关注写事件
        if (!channel_->IsWriting())
            channel_->EnableWriting();
    }
}

void TcpConnection::Shutdown()
{
    if (state_ == k_connected)
    {
        // 标记处在断开连接中
        SetState(k_disconnecting);
        loop_->RunInLoop(std::bind(&TcpConnection::ShutdownInLoop, this));
    }
}

void TcpConnection::ShutdownInLoop()
{
    loop_->AssertInLoopThread();
    // 优雅关闭连接
    if (!channel_->IsWriting())
        socket_->ShutdownWrite();
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

void TcpConnection::ConnDestroyed()
{
    loop_->AssertInLoopThread();
    assert(state_ == k_connected);
    SetState(k_disconnected);
    channel_->DisableAll();                     // 每断开一次连接，Channel::DisableAll()会被调用两次
    conn_callback_(shared_from_this());         // 注意这里是调用用户设置的conn_callback_回调

    loop_->RemoveChannel(channel_.get());       // 会调用Poller::RemoveChannel()，从自己的表中删除channel
}

void TcpConnection::HandleRead(Timestamp recv_time)
{
    int saved_errno = 0;
    ssize_t n = input_buffer_.ReadFd(channel_->GetFd(), &saved_errno);
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
    if (n > 0)
        msg_callback_(shared_from_this(), &input_buffer_, recv_time);
    else if (n == 0)              // 当读到的数据长度为0时，说明客户端主动断开了连接
        HandleClose();
    else
    {
        errno = saved_errno;
        std::cerr << "TcpConnection::handleRead\n";
        HandleError();
    }
}

void TcpConnection::HandleWrite()
{

}


void TcpConnection::HandleClose()
{
    loop_->AssertInLoopThread();
    std::cout << "WARN!TcpConnection::HandleClose state = " << state_ << std::endl;
    assert(state_ == k_connected);
    // 本Channel会不关注所有事件，即将被析构
    channel_->DisableAll();                        // 每断开一次连接，Channel::DisableAll()会被调用两次
    // 调用TcpServer绑定的函数
    close_callback_ (shared_from_this());

}

void TcpConnection::HandleError()
{
    // 获取错误信息
    int err = sockets::GetSocketError(socket_->GetFd());
    std::cerr << "TcpConnection::handleError [" << name_
              << "] - SO_ERROR = " << err << " " << std::endl;
}