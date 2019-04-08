#ifndef BBKGL_HTTPSERVER_H
#define BBKGL_HTTPSERVER_H

#include "TcpServer.h"
#include "EventLoop.h"
#include "Callbacks.h"

class HttpServer
{
public:
    HttpServer(EventLoop *loop, int port, int thread_num);

    // 服务器启动
    void Start();

    void OnConnection(const TcpConnectionPtr &conn);

    void OnMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp recv_time);

private:
    TcpServer server_;
};


#endif //BBKGL_HTTPSERVER_H
