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
// 也就是Channel::HandleEven()还没执行完，Channel对象就已经没了

/*------------------------------------2019.04.01 22:03更新-------------------------------------*/
/* 陈硕大佬之前并没有提这个问题，最终也没有说这个问题，只是在书上留下了这么一句话
 * P311倒数第二段: 思考并验证：如果用户不持有TcpConnectionPtr，那么TcpConnection对象究竟什么时候析构。。。
 * 答案是：执行完TcpConnection::ConnDestroyed()就会被析构。。。。。
 * 所以解决问题的关键就在于，如果在TcpConnection::ConnDestroyed()执行之前，执行到Channel::HandleEvent中的语句
 * 即：event_handling_ = false，这个语句执行一定要在TcpConnection::ConnDestroyed()执行之前！！！！！！
 *
 *
 * ～～～～～～～～～～～～～～～～～～～～～～～～解决方法如下～～～～～～～～～～～～～～～
 *
 * */
// 通过提前阅读后面的代码，解决了这个问题！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！
// 1.将原先的 TcpServer::RemoveConnection()分割成了两个函数：RemoveConnection()和RemoveConnectionInLoop()
// 2.实际的从TcpServer::connections_中移除指向TcpConnection对象指针的操作被放到了RemoveConnectionInLoop()中
// 3.TcpServer::RemoveConnectionInLoop()会将TcpConnection::ConnDestroyed函数放到EventLoop::QueueInLoop()中执行
// 4.而EventLoop::QueueInLoop()中的函数是先进入队列，因为没有创建新的线程则，调用EventLoop::QueueInLoop()仍然是IO线程
// 5.IO线程调用EventLoop::QueueInLoop()不会调用EventLoop::Wakeup()，只会操作push队列，所以退出EventLoop::QueueInLoop()函数
// 6.接着回到了TcpServer::RemoveConnection()函数，回到了TcpConnection::HandleClose()、TcpConnection::HandleRead()
// 7.最终回到了Channel::HandleEvent，然后执行语句：event_handling_ = false;，接着回到了EventLoop::Loop()中；
// 8.在EventLoop::Loop()中执行了EventLoop::DoPendingFunctors()，会执行进入队列的TcpConnection::ConnDestroyed()
// 9.TcpConnection::ConnDestroyed()函数执行结束后，TcpConnection对象就会被析构，因为引用计数已经为0了
// 10.同时智能指针成员指向的Channel对象也会被析构，因为此前event_handling_ = false被执行，至此完全且安全地断开了连接～～

/*------------------------------------2019.04.01 22:41更新-------------------------------------*/
// 才发现我误会陈硕大哥了。。。
// loop_->QueeuInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
// 我看成了loop_->RunInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
// 啊！！！！！！！！！！！！！！！！！！折腾了一下午 + 一晚上。。。。不过更熟悉整个流程了～～～～


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
               const char *data,
               ssize_t len)
{
    printf("OnMessage(): received %zd bytes from connection [%s]\n",
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