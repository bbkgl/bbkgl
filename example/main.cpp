#include <iostream>
#include "TcpServer.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"

void OnConn(const TcpConnectionPtr &conn)
{
    conn->Send("Hello!");         // 给新连接的客户端发送消息
}

void OnMessage(const TcpConnectionPtr &conn, Buffer *msg, Timestamp when)
{
    std::string s = msg->RetrieveAsString();
    std::cout << s << std::endl;
    conn->Send(s);
    conn->Close(5);
}

int main()
{
    EventLoop loop;
    InetAddress addr(2333);
    TcpServer server(&loop, addr);
    server.SetThreadNum(4);
    server.SetConnCallback(OnConn);
    server.SetMsgCallback(OnMessage);
    server.Start();

    loop.Loop();
    return 0;
}