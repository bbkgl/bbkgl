//
// Created by bbkgl on 19-3-27.
//

#include "EventLoop.h"
#include <iostream>
#include <poll.h>

// __thread变量每一个线程有一份独立实体，
// 各个线程的值互不干扰。可以用来修饰那些带有全局性且值可能变，但是又不值得用全局变量保护的变量。
__thread EventLoop* t_loop_in_this_thread = nullptr;

// EventLoop对象创建时，looping属性会被赋值为false，对象销毁时，会判断这个值是不是false
EventLoop::EventLoop()
        : looping_(false),
          thread_id_(std::this_thread::get_id())
{
    std::cout << "EventLoop created " << this << " in thread " << thread_id_ << std::endl;

    // 因为t_loop_in_this_thread一个线程是一份独立实体，初始值为空，如果一个线程创造了两个EventLoop对象
    // 那么创造了第二个对象的时候，t_loop_in_this_thread已经被第一个对象赋值，这时候就会报错，显示线程已经被占用
    if (t_loop_in_this_thread)
    {
        std::cout << "Another EventLoop " << t_loop_in_this_thread
                  << " exists in this thread " << thread_id_ << std::endl;
    }
    else
    {
        t_loop_in_this_thread = this;
    }
}

EventLoop::~EventLoop()
{

    assert(!looping_);
    t_loop_in_this_thread = nullptr;
}

void EventLoop::Loop()
{
    // 如果looping在对象创建后没有被改动（即仍然为false，表示Loop()函数没有运行过）
    assert(!looping_);

    // 判断当前线程是否是创建时的线程，如果是的话，“事件循环”启动
    AssertInLoopThread();
    // “事件循环”启动
    looping_ = true;

    // 当前的poll函数什么都不干，这里表示等待5秒后退出
    ::poll(nullptr, 0, 5*1000);

    // 循环结束，停止事件循环
    std::cout << "EventLoop " << this << " stop looping\n";
    looping_ = false;
}

void EventLoop::AbortNotInLoopThread()
{
    std::cout << "EventLoop::abortNotInLoopThread - EventLoop " << this
              << " was created in threadId_ = " << thread_id_
              << ", current thread id = " <<  std::this_thread::get_id() << std::endl;
}