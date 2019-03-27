#include <iostream>
#include <unistd.h>
#include <thread>
#include "EventLoop.h"

EventLoop *g_loop;

void ThreadFunc1()
{
    // 打印子线程的进程id和线程id
    std::cout << "Func(): pid = " << getpid() << " tid = " << std::this_thread::get_id() << std::endl;

    // loop对象在子线程中创建，并运行了loop循环
    EventLoop loop;
    loop.Loop();
}

void ThreadFunc2()
{
    g_loop->Loop();
}

// 测试1
int main()
{
    // 打印主线程的进程id和线程id
    std::cout << "Func(): pid = " << getpid() << " tid = " << std::this_thread::get_id() << std::endl;

    EventLoop loop;

    std::thread t(ThreadFunc1);
    t.join();

    // loop对象在主线程中创建，并运行了loop循环
    loop.Loop();

    return 0;
}

// 测试2
//int main()
//{
//    // 创建了一个事件循环对象
//    EventLoop loop;
//
//    // 让g_loop指针指向了loop对象
//    g_loop = &loop;
//
//    // 创建一个线程，运行了loop对象
//    // 这样会发生错误，因为loop对象是在主线程创建的，但是却在子线程被启动了Loop成员函数
//    std::thread t(ThreadFunc2);
//    t.join();
//    return 0;
//}