#include <cstdio>
#include "EventLoop.h"
#include "InetAddress.h"
#include "TcpServer.h"


void OnConnection(const TcpConnectionPtr &conn)
{
    if (conn->Connected())
    {
        printf("OnConnection(): {tid = %d} new connection [%s] from %s\n",
               std::this_thread::get_id(),
               conn->GetName().c_str(),
               conn->GetPeerAddress().ToHostPort().c_str());
    } else
    {
        printf("OnConnection(): {tid = %d} connection [%s] is down\n", std::this_thread::get_id(),
               conn->GetName().c_str());
    }
}

void OnMessage(const TcpConnectionPtr &conn,
               Buffer *data,
               Timestamp recv_time)
{
    std::string recv = data->RetrieveAsString();
    printf("OnMessage(): {tid = %d} received %zd bytes from connection [%s] at %s\n", std::this_thread::get_id(),
           data->ReadableBytes(), conn->GetName().c_str(), recv_time.toFormattedString().c_str());
    printf("OnMessage: [%s]\n", recv.substr(0, recv.length() - 1).c_str());

    conn->Send(recv);
}

int main()
{
    printf("test9_1: pid = %d\n", getpid());

    InetAddress listen_addr(2333);
    EventLoop loop;

    TcpServer server(&loop, listen_addr);
    server.SetConnCallback(OnConnection);
    server.SetMsgCallback(OnMessage);
    server.SetThreadNum(4);
    server.Start();

    loop.Loop();

    return 0;
}