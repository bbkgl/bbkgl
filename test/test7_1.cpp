#include "Acceptor.h"
#include "base/InetAddress.h"
#include "EventLoopThread.h"
#include "EventLoop.h"
#include "base/SocketsOpts.h"
#include "src/base/Timestamp.h"
#include <cstring>

/*
 * 按照陈硕书中303页要求实现的daytime服务器
 * 虽然我找了半天也不知道daytime服务器是什么。。。
 *
 * */

// 本函数首先由Acceptor::new_conn_callback_接收，然后通过Acceptor::HandleRead()一起
// 传给了Acceptor::accept_channel_中，最后在Loop()中poller_检测到事件，然后Acceptor::HandleRead()被调用
// 最后本函数被调用
void NewConn(int sockfd, const InetAddress &peer_addr)
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

int main()
{
    printf("test7_1(): pid = %d\n", getpid());

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