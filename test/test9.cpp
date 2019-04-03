#include <cstdio>
#include "EventLoop.h"
#include "base/InetAddress.h"
#include "TcpServer.h"


void OnConnection(const TcpConnectionPtr &conn)
{
    if (conn->Connected())
    {
        printf("OnConnection(): new connection [%s] from %s\n",
               conn->GetName().c_str(),
               conn->GetPeerAddress().ToHostPort().c_str());
    } else
    {
        printf("OnConnection(): connection [%s] is down\n",
               conn->GetName().c_str());
    }
}

void OnMessage(const TcpConnectionPtr &conn,
               Buffer *data,
               Timestamp recv_time)
{
    std::string recv = data->RetriveAsString();
    printf("OnMessage(): received %zd bytes from connection [%s] at %s\n",
           data->ReadableBytes(), conn->GetName().c_str(), recv_time.toFormattedString().c_str());
    printf("OnMessage: [%s]\n", recv.substr(0, recv.length() - 1).c_str());

    conn->Send(recv);
}

int main()
{
    printf("test9: pid = %d\n", getpid());

    InetAddress listen_addr(2333);
    EventLoop loop;

    TcpServer server(&loop, listen_addr);
    server.SetConnCallback(OnConnection);
    server.SetMsgCallback(OnMessage);
    server.Start();

    loop.Loop();

    return 0;
}