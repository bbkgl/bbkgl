#include <cstdio>
#include "EventLoop.h"
#include "EventLoopThread.h"
#include <thread>

void RunInThread()
{
    printf("RunInThread(): pid = %d, tid = %d\n", getpid(), std::this_thread::get_id());
}

int main()
{
    printf("test6: pid = %d, tid = %d\n", getpid(), std::this_thread::get_id());

    EventLoopThread loop_thread;
    // 在子线程中生成了一个EventLoop对象，并返回了指针
    EventLoop *loop = loop_thread.StartLoop();
    // 由于并非本线程生成的对象，所以打印的线程ID和刚刚打印的线程ID不一样
    // 执行顺序：loop->RunInLoop(RunInThread)--->QueueInLoop(cb)--->pending_functors_.push_back(cb)
    //         --->pending_functors_.push_back(cb)--->EventLoop::Wakeup()--->EventLoop::poller_.poll()
    //         --->EventLoop::DoPendingFunctors()--->RunInThread()
    // 上述过程，从loop->RunInLoop(RunInThread)------->EventLoop::Wakeup()都是由非IO线程执行的，经由EventLoop::Wakeup()
    // 唤醒EventLoop::poller_.poll()后，才回到IO线程，执行最后的RunInThread()
    loop->RunInLoop(RunInThread);
    sleep(1);


    // 由于并非本线程生成的对象，所以打印的线程ID和刚刚打印的线程ID不一样
    // 执行顺序：loop->RunAfter(2, RunInThread)--->EventLoop::RunAt()--->TimerQueue::AddTimer()
    //         --->TimerQueue::AddTimerInLoop()--->loop->RunInLoop(RunInThread)--->后续过程和上面的一样了就
    // 上述过程，从loop->RunAfter(2, RunInThread)------->EventLoop::Wakeup()都是由非IO线程执行的
    // 经由EventLoop::Wakeup()唤醒EventLoop::poller_.poll()后，才回到IO线程，执行最后的RunInThread()
    loop->RunAfter(2, RunInThread);
    sleep(3);

    // 同样非IO线程调用，要先经历唤醒才会最终退出。。。
    loop->Quit();

    /*
     * 在Wakeup()函数中打印了“WakeUp”，测试后确实打印了三次“WakeUp”
     * 所以就证实了loop->RunInLoop(RunInThread)、loop->RunAfter(2, RunInThread)、loop->Quit()都不是IO线程调用
     * 且Wakeup函数在非IO线程调用时一定会触发！
    */
    printf("test6 exited");
    return 0;
}