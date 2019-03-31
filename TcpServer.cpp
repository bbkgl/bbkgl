//
// Created by bbkgl on 19-3-31.
//

#include "TcpServer.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "base/SocketsOpts.h"
#include <cstdio>
#include <iostream>

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listen_addr) :
    loop_(loop),
    name_(listen_addr.ToHostPort()),
    acceptor_(new Acceptor(loop, listen_addr)),
    started_(false),
    next_conn_id_(1)               // 这是唯一标记连接的，最后用来生成一台服务器上所有连接TcpConnection的映射
{
    // NewConnection是个带参函数，回调的话需要设置占位符
    // 这里的调用过程是：acceptor_->SetNewConnetionCallback()--->Acceptor::Acceptor()--->
    // accept_channel_.SetReadCallback(Acceptor::HandleRead())--->Channel::Update()
    // 这里注意这里会进行注册，即最终注册到poller的事件表上，这也是为什么加上Channel::Update()的原因
    acceptor_->SetNewConnetionCallback(std::bind(&TcpServer::NewConnection, this,
                                                 std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer()
{

}

void TcpServer::Start()
{
    if (!started_)
        started_ = true;
    if (!acceptor_->Listenning())
        // 这里之前郁闷了好久，才想起来这里用的智能指针unique_ptr，所以不能直接传acceptor_
        // 使用loop_->RunInLoop()
        loop_->RunInLoop(std::bind(&Acceptor::Listen, acceptor_.get()));
//        acceptor_->Listen();
}

// 接收新连接
// 本函数会在Channel中执行。。
// 执行调用顺序：EventLoop::poller_.poll()--->Channel::HandleEvent()--->Acceptor::HandleRead()
// --->TcpServer::NewConnection()
void TcpServer::NewConnection(int sockfd, const InetAddress &peer_addr)
{
    // 判断是否IO线程调用
    loop_->AssertInLoopThread();
    // 格式化生成唯一标示连接TcpConnection的映射中的键值conn_name
    char buf[32];
    ++next_conn_id_;
    snprintf(buf, sizeof(buf), "#%d", next_conn_id_);
    std::string conn_name = name_ + buf;
    std::cout << "TcpServer::newConnection [" << name_
              << "] - new connection [" << conn_name
              << "] from " << peer_addr.ToHostPort() << std::endl;
    // 本地host
    InetAddress local_addr(sockets::GetLocalAddr(sockfd));

    // 构造新的连接对象TcpConnection
    TcpConnectionPtr conn(
            new TcpConnection(loop_, conn_name, sockfd, local_addr, peer_addr));
    // 生成映射
    connections_[conn_name] = conn;
    // 设置用户传入的回调
    conn->SetConnCallback(conn_callback_);
    conn->SetMsgCallback(msg_callback);
    // 建立连接以后，conn->ConnEstablished()首先进行标记，然后会区poller中注册可读事件，最后调用用户设定的回调函数
    conn->ConnEstablished();
}