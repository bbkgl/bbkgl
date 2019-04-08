#include "HttpServer.h"
#include "InetAddress.h"

HttpServer::HttpServer(EventLoop *loop, int port, int thread_num) :
    server_(loop, *(new InetAddress(port)))
{
    server_.SetThreadNum(thread_num);
    server_.SetConnCallback(std::bind(&HttpServer::OnConnection, this));
    server_.SetConnCallback(std::bind(&HttpServer::OnMessage, this));
}

void HttpServer::Start()
{
    server_.Start();
}

void HttpServer::OnConnection(const TcpConnectionPtr &conn)
{

}

void HttpServer::OnMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp recv_time)
{

}