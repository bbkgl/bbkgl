#include "EventLoop.h"
#include <cstdio>

// 全局EventLoop指针
EventLoop *g_loop;
int g_flag = 0;

// 打印完信息后，退出
void Run4()
{
    printf("run4(): pid = %d, flag = %d\n", getpid(), g_flag);
    g_loop->Quit();
}

// 打印完信息后，3秒后执行Run4()函数
void Run3()
{
    printf("run3(): pid = %d, flag = %d\n", getpid(), g_flag);
    g_loop->RunAfter(3, Run4);
    g_flag = 3;
}

// 打印完信息后，将Run3()函数加入到执行队列中去
void Run2()
{
    printf("Run2(): pid = %d, flag = %d\n", getpid(), g_flag);
    g_loop->QueueInLoop(Run3);
}

// 打印完后，直接执行Run2()函数
void Run1()
{
    g_flag = 1;
    printf("run1(): pid = %d, flag = %d\n", getpid(), g_flag);
    g_loop->RunInLoop(Run2);
    g_flag = 2;
}

int test5()
{
    printf("test5(): pid = %d, flag = %d\n", getpid(), g_flag);

    EventLoop loop;
    g_loop = &loop;

    // 2s后执行Run1()函数
    loop.RunAfter(2, Run1);
    loop.Loop();

    printf("test5(): pid = %d, flag = %d\n", getpid(), g_flag);

    return 0;
}
