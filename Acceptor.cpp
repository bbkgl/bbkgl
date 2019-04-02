//
// Created by bbkgl on 19-3-30.
//

#include "Acceptor.h"
#include "EventLoop.h"
#include "base/InetAddress.h"
#include "base/SocketsOpts.h"

/*
 * 分析整个过程，可以看到，其实Acceptor本身是不处理事件的，也都是交给了某个channel去处理
 * 1.Acceptor通过所在的EventLoop(loop_)和用来监听的fd构造了一个对象accept_channel_
 * 2.然后Acceptor通过回调函数把自己接收请求的函数传给了accept_channel_，
 * 3.接着调用Accepto::listen()启动监听，其实就是让文件描述符Accepto::Socket::sockfd_负责监听
 * 4.此时EventLoop::Loop()启动循环，EventLoop::poller_负责接收请求
 * 5.如果来了新的连接，根据之前监听的fd就能唯一确定了之前构造的对象accept_channel_
 * 6.accept_channel_会调用传入的回调函数（Acceptor::HandleRead()）接收新的连接
 *
 * 总的来说，服务器中所有发生的事件，读、写、接受连接，错误处理，都是交给channel处理的！！！
 *
 * */

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listen_addr) :
    loop_(loop),
    accept_socket_(sockets::CreateNonblockingOrDie()),
    accept_channel_(loop, accept_socket_.GetFd()),
    listenning_(false)
{
    // 设置端口复用
    accept_socket_.SetReUseAddr(true);
    // 绑定
    accept_socket_.BindAddress(listen_addr);
    // 将读取用户请求的操作交给Channel去处理
    accept_channel_.SetReadCallback(std::bind(&Acceptor::HandleRead, this));   // 注册可读事件
}

void Acceptor::Listen()
{
    // 判断是否IO线程调用
    loop_->AssertInLoopThread();
    // 开始监听
    listenning_ = true;
    accept_socket_.Listen();
    // 让自己的Channel关注可读事件，此前已经把自己读取请求的操作回调给了channel
    accept_channel_.EnableReading();
}

void Acceptor::HandleRead()
{
    // 判断是否是IO线程调用
    loop_->AssertInLoopThread();
    // 用来存储客户端addr的变量
    InetAddress peer_addr(0);

    // 接收到客户端请求
    int connfd = accept_socket_.Accept(&peer_addr);
    if (connfd >= 0)
    {
        // 给客户端发送消息
        if (new_conn_callback_)
            new_conn_callback_(connfd, peer_addr);
        else
            sockets::Close(connfd);
    }
}