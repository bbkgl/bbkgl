#include <cstdio>
#include <Callbacks.h>
#include "EventLoop.h"
#include "InetAddress.h"
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

/*
 * 这个write_complete__callback_ = nullptr;非常重要！！！！！！
 * 陈硕大佬的书中没有讲这个问题，没有上面这句，会陷入死循环。可以跟踪一下整个调用过程。
 * 系统调用write_complete__callback_，也就是chargen服务器中的OnWriteComplete()
 * OnWriteComplete()中继续调用TcpConnection::Send()，然后又会回到这里把write_complete__callback_放入到
 * 队列里，于是用户传入的OnWriteComplete()会继续执行，又会调用TcpConnection::Send()。。。。
 * 陷入了死循环。。。。
 *
 * */

void OnMessage(const TcpConnectionPtr &conn,
               Buffer *data,
               Timestamp recv_time)
{
    std::string recv = data->RetrieveAsString();
    printf("OnMessage(): received %zd bytes from connection [%s] at %s\n",
           data->ReadableBytes(), conn->GetName().c_str(), recv_time.toFormattedString().c_str());
    printf("OnMessage: [%s]\n", recv.substr(0, recv.length() - 1).c_str());
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