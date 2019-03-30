#include "Acceptor.h"
#include "base/InetAddress.h"
#include "EventLoopThread.h"
#include "EventLoop.h"
#include "base/SocketsOpts.h"

// 本函数首先由Acceptor::new_conn_callback_接收，然后通过Acceptor::HandleRead()一起
// 传给了Acceptor::accept_channel_中，最后在Loop()中poller_检测到事件，然后Acceptor::HandleRead()被调用
// 最后本函数被调用
void NewConn(int sockfd, const InetAddress &peer_addr)
{
    printf("NewConn(): accepted a new connection from %s.\n",
           peer_addr.ToHostPort().c_str());
    // 给客户端回话
    write(sockfd, "How are you?\n", 13);
    // 关闭套接字
    sockets::Close(sockfd);
}

int main()
{
    printf("test7(): pid = %d\n", getpid());

    // 构造一个sockaddr地址的封装对象
    InetAddress listen_addr(2333);

    // 不谈了
    EventLoop loop;

    // 接收器
    Acceptor acceptor(&loop, listen_addr);

    // 设置回调函数
    acceptor.SetNewConnetionCallback(NewConn);
    // 开始监听
    acceptor.Listen();

    // 启动循环
    loop.Loop();

    return 0;
}