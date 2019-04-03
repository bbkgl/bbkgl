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
    conn->SetWriteCompleteCallback(write_complete_callback_);
    conn->SetCloseCallback(std::bind(&TcpServer::RemoveConnection, this, std::placeholders::_1));
    // 建立连接以后，conn->ConnEstablished()首先进行标记，然后会区poller中注册可读事件，最后调用用户设定的回调函数
    conn->ConnEstablished();
}

void TcpServer::RemoveConnection(const TcpConnectionPtr &conn)
{
    loop_->RunInLoop(std::bind(&TcpServer::RemoveConnectionInLoop, this, conn));
}

void TcpServer::RemoveConnectionInLoop(const TcpConnectionPtr &conn)
{
    loop_->AssertInLoopThread();
    std::cout << "TcpServer::removeConnection [" << name_
              << "] - connection " << conn->GetName() << std::endl;
    size_t n = connections_.erase(conn->GetName());
    assert(n == 1);

    EventLoop *io_loop = conn->GetLoop();

    io_loop->QueueInLoop(std::bind(&TcpConnection::ConnDestroyed, conn));
//    printf("%ld\n", conn.use_count());
    // 这里要特别注意！！！！特别坑！！！
    // 因为新建的TcpConnection对象都是在TcpServer::NewConnection()中建立的，如果TcpServer::connections_
    // 进行了erase操作，删除了指向TcpConnection对象的指针，那向TcpConnection对象的引用计数会减一，仅仅剩下了
    // conn来指向这个对象，当本函数运行结束，退出内存后conn指针也会被抹掉，conn本身就是智能指针，而且传的是自己的引用
    // ，他被抹掉之前，会调用TcpConnection对象的析构函数，这也是为什么TcpConnection对象的析构函数会打印出析构函数
    // 中的的信息。一旦TcpConnection对象被析构，那么TcpConnection对象中channel_智能指针对象被析构，那么channel_指
    // 向的对象也会被析构，而Channel::HandleEven()函数还没执行到"event_handling_ = false;"，其函数所在对象
    // 就已经被析构了，所以当Channel对象的析构函数被执行时Channel::event_handling_的值依旧是true，造成了assert()报错！！！


    /*------------------------------------2019.04.01 22:03更新-------------------------------------*/
    // 通过提前阅读后面的代码，解决了这个问题！！！！
    // 1.将原先的 TcpServer::RemoveConnection()分割成了两个函数：RemoveConnection()和RemoveConnectionInLoop()
    // 2.实际的从TcpServer::connections_中移除指向TcpConnection对象指针的操作被放到了RemoveConnectionInLoop()中
    // 3.TcpServer::RemoveConnectionInLoop()会将TcpConnection::ConnDestroyed函数放到EventLoop::QueueInLoop()中执行
    // 4.而EventLoop::QueueInLoop()中的函数是先进入队列，因为没有创建新的线程则，调用EventLoop::QueueInLoop()仍然是IO线程
    // 5.IO线程调用EventLoop::QueueInLoop()不会调用EventLoop::Wakeup()，只会操作push队列，所以退出EventLoop::QueueInLoop()函数
    // 6.接着回到了TcpServer::RemoveConnection()函数，回到了TcpConnection::HandleClose()、TcpConnection::HandleRead()
    // 7.最终回到了Channel::HandleEvent，然后执行语句：event_handling_ = false;，接着回到了EventLoop::Loop()中；
    // 8.在EventLoop::Loop()中执行了EventLoop::DoPendingFunctors()，会执行进入队列的TcpConnection::ConnDestroyed()
    // 9.TcpConnection::ConnDestroyed()函数执行结束后，TcpConnection对象就会被析构，因为引用计数已经为0了，至此完全断开连接～～

    /*------------------------------------2019.04.01 22:41更新-------------------------------------*/
    // 才发现我误会陈硕大哥了。。。
    // loop_->QueeuInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    // 我看成了loop_->RunInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    // 啊！！！！！！！！！！！！！！！！！！折腾了一下午 + 一晚上。。。。不过更熟悉整个流程了～～～～
}