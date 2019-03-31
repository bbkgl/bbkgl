#include "EventLoop.h"
#include "TcpServer.h"
#include "base/InetAddress.h"
#include <cstdio>

void OnConnection(const TcpConnectionPtr &conn)
{
    if (conn->Connected())
        printf("OnConnection(): new connection [%s] from %s\n",
               conn->GetName().c_str(), conn->GetPeerAddress().ToHostPort().c_str());
    else
        printf("OnConnection(): connection [%s] is down\n",
               conn->GetName().c_str());
}

void OnMessage(const TcpConnectionPtr &conn, const char *data, ssize_t len)
{
    printf("OnMessage(): received %zd bytes from connection [%s]\n",
           len, conn->GetName().c_str());
}

int main()
{
    printf("test8(): pid = %d\n", getpid());

    InetAddress listen_addr(2333);
    EventLoop loop;

    TcpServer server(&loop, listen_addr);

    /*
     * OnMessage()和OnConnection()都是在TcpServer::NewConnection()中被传入到server中
     * 但是调用不一样，监听开始后，EventLoop.poller_.Poll()会委托内核检测事件，检测到可读事件后，会返回给EventLoop
     * 活跃的channels，其中就包括了Acceptor::accept_channel_，首先在EventLoop::Loop()中会接收到这个Acceptor::accept_channel_
     * 然后Acceptor::accept_channel_->HandleEvent()会被调用，然后就会调用到
     *
     *
     * */

    // OnMessage()函数传入过程：TcpServer::SetConnCallback()
    // OnMessage()函数调用过程：
    server.SetMsgCallback(OnMessage);
    server.SetConnCallback(OnConnection);
    server.Start();

    loop.Loop();

    return 0;
}
