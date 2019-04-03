#include <cstdio>
#include "EventLoop.h"
#include "base/InetAddress.h"
#include "TcpServer.h"

std::string msg1;
std::string msg2;

void OnConnection(const TcpConnectionPtr &conn)
{
    if (conn->Connected())
    {
        printf("OnConnection(): new connection [%s] from %s\n",
               conn->GetName().c_str(),
               conn->GetPeerAddress().ToHostPort().c_str());
        conn->Send(msg1);
        conn->Send(msg2);
        conn->Shutdown();
        /*
         * 连续两次发送数据，会导致EAGAIN错误。。。
         * 因为发送msg1以后，msg1的数据量过大，本身msg1剩余的数据就得通过Buffer发送了，也就是说
         * 系统缓冲区已经满了，继续写的话，会导致write强行往已经满了的缓冲区写入数据，这样就会报EAGAIN错误
         *
         *
         * 所以一般是建议一次性发送，就算数据很长。。。。。。。。
         *
         * */
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
    std::string recv = data->RetrieveAsString();
    printf("OnMessage(): received %zd bytes from connection [%s] at %s\n",
           data->ReadableBytes(), conn->GetName().c_str(), recv_time.toFormattedString().c_str());
    printf("OnMessage: [%s]\n", recv.substr(0, recv.length() - 1).c_str());

    conn->Send(recv);
}

int main()
{
    printf("test10: pid = %d\n", getpid());

    msg1.resize(10000000, '1');
    msg2.resize(100000, '2');

    InetAddress listen_addr(2333);
    EventLoop loop;

    TcpServer server(&loop, listen_addr);
    server.SetConnCallback(OnConnection);
    server.SetMsgCallback(OnMessage);
    server.Start();

    loop.Loop();

    return 0;
}