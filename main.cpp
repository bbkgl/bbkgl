#include <cstdio>
#include "EventLoop.h"
#include "base/InetAddress.h"
#include "TcpServer.h"

/*-------------------------本用例执行不是完全成功，原因如下-----------------------*/

// 因为新建的TcpConnection对象都是在TcpServer::NewConnection()中建立的，如果TcpServer::connections_
// 进行了erase操作，删除了指向TcpConnection对象的指针，那向TcpConnection对象的引用计数会减一，仅仅剩下了
// conn来指向这个对象，当本函数运行结束，退出内存后conn指针也会被抹掉，conn本身就是智能指针，他被抹掉之前，会
// 调用TcpConnection对象的析构函数，这也是为什么TcpConnection对象的析构函数会打印出析构函数中的的信息。一旦
// TcpConnection对象被析构，那么TcpConnection对象中channel_智能指针对象被析构，那么channel_指向的对象也会
// 被析构，而Channel::HandleEven()函数还没执行到"event_handling_ = false;"，其函数所在对象就已经被析构了，
// 所以当Channel对象的析构函数被执行时Channel::event_handling_的值依旧是true，造成了assert()报错！！！

/*也就是Channel::HandleEven()还没执行完，Channel对象就已经没了。。。。*/

void OnConnection(const TcpConnectionPtr &conn)
{
    if (conn->Connected())
    {
        printf("onConnection(): new connection [%s] from %s\n",
               conn->GetName().c_str(),
               conn->GetPeerAddress().ToHostPort().c_str());
    } else
    {
        printf("onConnection(): connection [%s] is down\n",
               conn->GetName().c_str());
    }
}

void OnMessage(const TcpConnectionPtr &conn,
               const char *data,
               ssize_t len)
{
    printf("onMessage(): received %zd bytes from connection [%s]\n",
           len, conn->GetName().c_str());
}

int main()
{
    printf("test8: pid = %d\n", getpid());

    InetAddress listen_addr(2333);
    EventLoop loop;

    TcpServer server(&loop, listen_addr);
    server.SetConnCallback(OnConnection);
    server.SetMsgCallback(OnMessage);
    server.Start();

    loop.Loop();

    return 0;
}