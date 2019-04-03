
#include "Acceptor.h"
#include "base/InetAddress.h"
#include "EventLoopThread.h"
#include "EventLoop.h"
#include "base/SocketsOpts.h"
#include "src/base/Timestamp.h"
#include <cstring>

/*
 * 2019年3月30日20:45
 * 按照陈硕的书303页实现的同时监听两个port的服务器，每个port发送不同的字符串。
 * 可以实现功能，但暂时不知道如何解决非IO线程调用Acceptor::Listen()的问题。
 * 因为如果主线程直接调用accept2.Listen()，最后就是在Listen()函数中执行loop_->AssertInLoopThread()，
 * 由于loop_是子线程中创建，则会检测到是非IO线程调用。。。。
 *
 * 2019年3月30日21:41
 * 成功实现陈硕的书303页第二个练习，解决了之前的“非IO线程调用Acceptor::Listen()的问题”。
 * 将accept2.Listen()函数利用std::bind()放入到EventLoop::RunInLoop()中执行，这样最终执行
 * Listen()函数的就是IO线程，这个在test6中有解释。
 *
 * */

// 第一个回复客户端的回调函数
// 本函数首先由Acceptor::new_conn_callback_接收，然后通过Acceptor::HandleRead()一起
// 传给了Acceptor::accept_channel_中，最后在Loop()中poller_检测到事件，然后Acceptor::HandleRead()被调用
// 最后本函数被调用
void NewConn1(int sockfd, const InetAddress &peer_addr)
{
    Timestamp time;
    char buf[50];
    sprintf(buf, "(Time: %s)---", time.now().toFormattedString().c_str());
    printf("%sNewConn(): accepted a new connection from %s.\n",
           buf, peer_addr.ToHostPort().c_str());
    // 给客户端回话
    strcat(buf, "How are you?\n");
    write(sockfd, buf, sizeof(buf));
    // 关闭套接字
    sockets::Close(sockfd);
}

// 第二个回复客户端的回调函数
// 本函数首先由Acceptor::new_conn_callback_接收，然后通过Acceptor::HandleRead()一起
// 传给了Acceptor::accept_channel_中，最后在Loop()中poller_检测到事件，然后Acceptor::HandleRead()被调用
// 最后本函数被调用
void NewConn2(int sockfd, const InetAddress &peer_addr)
{
    Timestamp time;
    char buf[50];
    sprintf(buf, "(Time: %s)---", time.now().toFormattedString().c_str());
    printf("%sNewConn(): accepted a new connection from %s.\n",
           buf, peer_addr.ToHostPort().c_str());
    // 给客户端回话
    strcat(buf, "How do you do?\n");
    write(sockfd, buf, sizeof(buf));
    // 关闭套接字
    sockets::Close(sockfd);
}


int main()
{
    printf("test7_1(): pid = %d\n", getpid());

    // 根据端口构造两个sockaddr地址的封装对象，一个给线程1，一个给线程2
    InetAddress listen_addr1(2333);
    InetAddress listen_addr2(1234);

    // 构造两个EventLoop对象，一个在主线程创建，一个在子线程创建
    EventLoop loop1;
    EventLoop *loop2 = (new EventLoopThread())->StartLoop();

    // 两个接收器
    Acceptor acceptor1(&loop1, listen_addr1);
    Acceptor acceptor2(loop2, listen_addr2);

    // 设置回调函数
    acceptor1.SetNewConnetionCallback(NewConn1);
    acceptor2.SetNewConnetionCallback(NewConn2);

    // 开始监听
    acceptor1.Listen();
    // 这样能保证调用Listen()函数的是创建loop2对象的线程，具体解释见test6
    loop2->RunInLoop(std::bind(&Acceptor::Listen, &acceptor2));

    // 启动循环
    loop1.Loop();

    return 0;
}