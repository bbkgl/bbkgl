#include <cstdio>
#include "EventLoop.h"
#include "base/InetAddress.h"
#include "TcpServer.h"

std::string msg;

void OnConnection(const TcpConnectionPtr &conn)
{
    if (conn->Connected())
    {
        printf("OnConnection(): new connection [%s] from %s\n",
               conn->GetName().c_str(),
               conn->GetPeerAddress().ToHostPort().c_str());
        conn->Send(msg);
    } else
    {
        printf("OnConnection(): connection [%s] is down\n",
               conn->GetName().c_str());
    }
}

void OnWriteComplete(const TcpConnectionPtr &conn)
{
    conn->Send(msg);
    printf("OnWriteComplete\n");
}

void OnMessage(const TcpConnectionPtr &conn,
               Buffer *data,
               Timestamp recv_time)
{
    std::string recv = data->RetrieveAsString();
    printf("OnMessage(): received %zd bytes from connection [%s] at %s\n",
           data->ReadableBytes(), conn->GetName().c_str(), recv_time.toFormattedString().c_str());
    printf("OnMessage: [%s]\n", recv.substr(0, recv.length() - 2).c_str());
}

int main()
{
    printf("test11: pid = %d\n", getpid());

    std::string line;

    for (int i = 33; i < 127; i++)
        line.push_back(static_cast<char>(i));

    line += line;

    for (size_t i = 0; i < 127-33; ++i)
        msg += line.substr(i, 72) + "\n";

    InetAddress listen_addr(2333);
    EventLoop loop;

    TcpServer server(&loop, listen_addr);
    server.SetConnCallback(OnConnection);
    server.SetMsgCallback(OnMessage);
    server.SetWriteCompleteCallback(OnWriteComplete);
    server.Start();

    loop.Loop();

    return 0;
}