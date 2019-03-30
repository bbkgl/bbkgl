#include "EventLoop.h"
#include <cstdio>
#include <thread>

int cnt = 0;
EventLoop *g_loop;         // 全局EventLoop指针，用于在外线程控制EventLoop对象退出

// 打印当前线程ID和时间
void PrintId()
{
    printf("pid = %d, tid = %d\n", getpid(), std::this_thread::get_id());
    printf("now %s\n", Timestamp::now().toString().c_str());
}

// 随着定时器传入loop的函数
void Print(const char *msg)
{
    // 打印传入的时间和消息
    printf("%d:\nmsg %s %s\n", cnt + 1, Timestamp::now().toString().c_str(), msg);
    // 如果回调函数触发了20次
    if (++cnt == 20)
    {
        g_loop->Quit();
    }
}

int main()
{
    // 打印当前线程ID和时间
    PrintId();
    EventLoop loop;
    g_loop = &loop;

    // 开始注册定时器事件
    printf("test4\n\n");
    // 1秒后触发回调函数
    loop.RunAfter(1, std::bind(Print, "once1"));
    // 1.5秒后触发回调函数
    loop.RunAfter(1.5, std::bind(Print, "once1.5"));
    // 2.5秒后触发回调函数
    loop.RunAfter(2.5, std::bind(Print, "once2.5"));
    // 3.5秒后触发回调函数
    loop.RunAfter(3.5, std::bind(Print, "once3.5"));
    // 每2秒触发回调函数
    loop.RunEvery(2, std::bind(Print, "every2"));
    // 每3秒触发回调函数
    loop.RunEvery(3, std::bind(Print, "every3"));

    loop.Loop();

    printf("main loop exits");

    sleep(1);

    return 0;
}