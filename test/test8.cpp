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
     * OnConnection()是在TcpServer::NewConnection()中被传入到server中，然后在TcpServer::NewConnection()被调用时
     * 经过操作：conn->SetConnCallback(conn_callback_)，传入到了TcpConnection::conn_callback_中
     * 最后连接建立后，在TcpServer::NewConnection()被调用后，又调用了TcpConnection::ConnEstablished()
     * 随即调用conn_callback_(shared_from_this())，最终才调用到了OnConnection()函数
     * */
    server.SetConnCallback(OnConnection);

    /*
     * OnMessage()是在TcpServer::NewConnection()中被传入到server中，然后在TcpServer::NewConnection()被调用时
     * 经过操作：conn->SetMsgCallback(msg_callback);，传入到了TcpConnection::msg_callback_中
     * OnMessage()会由TcpConnection::HandleRead()调用，但是TcpConnection::HandleRead()被传入到了
     * TcpConnection::channel_中，在此之前调用TcpConnection::ConnEstablished()时，已经注册了TcpConnection::channel_事件
     * 最终是EventLoop::poller_.Poll()检测到了TcpConnection::channel_触发读事件，最终才调用了OnMessage()
     *
     * */
    server.SetMsgCallback(OnMessage);

    // 开始监听
    server.Start();

    loop.Loop();

    return 0;
}
